#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#define _USE_MATH_DEFINES
#include <math.h>

#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <cassert>
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "externals/DirectXTex/DirectXTex.h"

#include <fstream>
#include <sstream>

#include <wrl.h>
#include <random>
#include <numbers>

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include "D3DResourceLeakChecker.h"
#include "MyMath.h"

//-------------------------------------
//BlendMode
//-------------------------------------

enum BlendMode {
	//ブレンドなし
	kBlendModeNone,
	//通常行αブレンド。デフォルト
	kBlendModeNormal,
	//加算
	kBlendModeAdd,
	//減算
	kBlendModeSubtract,
	//乗算
	kBlendModeMultily,
	//スクリーン
	kBlendModeScreen,
	//利用してはいけない
	kCountOfBlendMode,
};

typedef struct {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
}VertexData;

typedef struct {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;

}Material;

typedef struct {
	Matrix4x4 WVP;
	Matrix4x4 World;
}TransformMatrix;

//Transform情報を作る
typedef struct
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
}Transform;

//MaterialData構造体
typedef struct {
	std::string textureFilePath;
}MaterialData;

//ModelData構造体
typedef struct {
	std::vector<VertexData> vertices;
	MaterialData material;
}ModelData;

struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	// 生存可能な時間
	float lifeTime;
	// 発生してからの経過時間
	float currentTime;
};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

//-------------------------------------
//DepthStencilTextureを作る
//-------------------------------------

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const int32_t width, const uint32_t height) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;//Textureの幅
	resourceDesc.Height = height;//Textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//奥行きor配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//２次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue, //Clear最適値
		IID_PPV_ARGS(&resource)//作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}

//-------------------------------------
//特定のインデックスのDescriptorHandleを取得する
//-------------------------------------
//CPUver.
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, const uint32_t descriptorSize, const uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

//GPUver.
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, const uint32_t descriptorSize, const uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

//-------------------------------------
//mtlファイルを読み込む関数
//-------------------------------------

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要になる変数の宣言
	//構築するマテリアルデータ
	MaterialData materialData;
	//ファイルから読んだ1行を格納するもの
	std::string line;

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	//とりあえず開けなかったら止める
	assert(file.is_open());

	//3.実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifilerに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

//-------------------------------------
//objファイルを読み込む関数
//-------------------------------------

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	//1.必要となる変数の宣言
	ModelData modelData;;//構築するModelData
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャの座標
	std::string line;//ファイルから読んだ一行を格納するもの

	//2/ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());//開けなかったら止める

	//3.ファイルを読み、ModelDataを構築
	while (std::getline(file, line)) {
		//streamから１行読んでstd::stringに格納する関数。whileでファイルの最後まで１行ずつ読み込んでいく
		std::string identifier;
		//文字列を分析しながら読むためのクラス、空白を区切り文字として読んでいくことができる
		std::istringstream s(line);
		//先頭の識別子を読む
		s >> identifier;

		//identifierに応じた処理
		//"v":頂点位置
		//"vt":頂点テクスチャ座標
		//"vn":頂点法線
		//"f":面
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			//position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			//normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			//三角形を作る
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されてるので、　分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					//区切りでインデックスを読んでいく
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				
				texcoord.y = 1.0f - texcoord.y;
				
				triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイル名を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

void SetBlendState(D3D12_BLEND_DESC& blendDesc, BlendMode blendMode) {

	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//ブレンドを有効にするか無効にするかの指定
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	switch (blendMode)
	{
	case kBlendModeNone:
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		break;

	case kBlendModeNormal:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;

	case kBlendModeAdd:
		//加算合成 
		//Result=SrcColor*SrcAlpha+DestColor
		//ピクセルシェーダーを出力するRGB値に対して実行する操作を指定する
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//SrcBlend操作とDestBlend操作を組み合わせる方法を定義する
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;

	case kBlendModeSubtract:
		//減算合成
		//Result=DescColor-SrcColor*SrcAlpha
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;

	case kBlendModeMultily:
		//乗算合成
		//Result=SrcColor*DestColor
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;

	case kBlendModeScreen:
		//スクリーン合成
		//Result=(1-DestColor)*SrcColor+DestColor
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;

	default:
		break;
	}

	//α値ブレンド設定。基本的に使わないのでこのまま

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

}

/// <summary>
/// パーティクル一つを生成する
/// </summary>
/// <param name="randomEnging"></param>
/// <returns></returns>
Particle MakeNewParticle(std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 5.0f);

	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	// 位置と速度を[-1,1]でランダムに初期化
	particle.transform.translate = { distribution(randomEngine),distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine),distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distribution(randomEngine),distribution(randomEngine), distribution(randomEngine) ,1.0f };
	// 1~3秒の間生存するようにする
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

static Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(Microsoft::WRL::ComPtr<ID3D12Device> device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) {
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));
	return pipelineState;
}

//-------------------------------------
//main関数
//-------------------------------------

//windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//ポインタ
	D3DResourceLeakChecker* leakChecke = nullptr;
	Input* input = nullptr;
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;


	//-------------------------------------
	// リークチェッカー
	//-------------------------------------

	leakChecke->~D3DResourceLeakChecker();

	//-------------------------------------
	// WinAppAPIの初期化
	//-------------------------------------

	winApp = new WinApp();
	winApp->Initialize();

	//-------------------------------------
	// 入力の初期化
	//-------------------------------------

	input = new Input();
	input->Initialize(winApp);

	//-------------------------------------
	// DirectXの初期化
	//-------------------------------------

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);


#ifdef _DEBUG

	//-------------------------------------
	//エラーと警告の抑制
	//-------------------------------------

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(dxCommon->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}
#endif

	//-------------------------------------
	//ここからPSOの作成
	//-------------------------------------

	//-------------------------------------
	//DescriptorRangeを作成
	//-------------------------------------
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;//0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1;//数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算

	//-------------------------------------
	//RootSignatureを作成
	//-------------------------------------
	//ShaderとResourceをどのように関連付けるかを示したオブジェクト

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成。複数設定できるので配列。今回はPixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;// Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);// Tableで利用する数

	//-------------------------------------
	//DescriptorTableを作成
	//-------------------------------------
	//DescriptorRangeをまとめたもの
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);//Tableで利用する数

	//-------------------------------------
	//Samplerの設定
	//-------------------------------------

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	descriptionRootSignature.pParameters = rootParameters; //ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); //配列の長さ

	//シリアライズしてバイナリする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//バイナリを元に作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));


	//-------------------------------------
	//InputLayoutを作成
	//-------------------------------------
	//VertexShaderへ渡す頂点データがどのようなものかを指定するオブジェクト

	//InputLayoutの設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//-------------------------------------
	//BlendStateを作成
	//-------------------------------------
	//PixelShaderからの出力を画面にどのように書き込むかを設定する項目

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	BlendMode blendMode = kBlendModeNormal;

	SetBlendState(blendDesc, blendMode);

	//-------------------------------------
	//RasterizerStateを作成
	//-------------------------------------
	//Rasterizerに対する設定

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	//裏面（時計回り）を表示しない（ピクセルにしない）
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//-------------------------------------
	//ShaderをCompilerする
	//-------------------------------------

	//Shaderをコンパイラする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);


	//-------------------------------------
	//DepthStencilState
	//-------------------------------------

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みを行わない
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	//-------------------------------------
	//PSOを生成する
	//-------------------------------------

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayer
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;//depthStencilState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（気にしない）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//-------------------------------------
	//ModelDataを使ったResourceの作成
	//-------------------------------------

	//モデル読み込み
	ModelData modelData = LoadObjFile("resource", "plane.obj");
	//頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//１頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//-------------------------------------
	//矩形を描画するためのVertexResource
	//-------------------------------------

	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);

	//-------------------------------------
	//矩形を描画するためのVertexBufferView
	//-------------------------------------

	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭アドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点６つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//１頂点分のサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//-------------------------------------
	//矩形の頂点データを設定する
	//-------------------------------------

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//１枚目の三角形
	//左下

	vertexDataSprite[0].position = { 0.0f,360.f,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	//左上
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };

	//右下
	vertexDataSprite[2].position = { 640.f,360.f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };

	//右上
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };


	//-------------------------------------
	//矩形描画のindex版のResourceを作成
	//-------------------------------------

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	//Viewを作成
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//-------------------------------------
	//インデックスリソースにデータを書き込む
	//-------------------------------------
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//左下
	indexDataSprite[0] = 0;
	//左上
	indexDataSprite[1] = 1;
	//右下
	indexDataSprite[2] = 2;
	//左上
	indexDataSprite[3] = 1;
	//右上
	indexDataSprite[4] = 3;
	//右下
	indexDataSprite[5] = 2;

	//-------------------------------------
	//Material用のResourceを作る
	//-------------------------------------

	//マテリアル用のリソースを作る。今回はcolor１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込み
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	//単位行列で初期化
	materialData->uvTransform = MyMath::MakeIdentity4x4();

	//-------------------------------------
	//Splite用のマテリアルを作成して利用する
	//-------------------------------------

	//Sprite用のパーティクルマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//今回は白を書き込み
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = false;
	//単位行列で初期化
	materialDataSprite->uvTransform = MyMath::MakeIdentity4x4();

	//-------------------------------------
	//TransformationMatrix用のResourceを作る
	//-------------------------------------

	//WVP用のリソースを作る。Matrix4x4　１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformMatrix));
	//データを書き込む
	TransformMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	wvpData->WVP = MyMath::MakeIdentity4x4();
	wvpData->World = MyMath::MakeIdentity4x4();

	//-------------------------------------
	// Instancing用Resource
	//-------------------------------------

	// インスタンス最大数
	const uint32_t kNumMaxInstance = 10;

	// Instancing用のTransformationMatrixリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = dxCommon->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);

	// 書き込むためのアドレスを取得
	ParticleForGPU* instancingData = nullptr;
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));

	// 単位行列を書き込んでおく
	// 色はとりあえず白
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData[index].WVP = MyMath::MakeIdentity4x4();
		instancingData[index].World = MyMath::MakeIdentity4x4();
		instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//-------------------------------------
	//Transformを使ってCBufferを更新する
	//-------------------------------------
	//Transform変数を作る
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//-------------------------------------
	//uvTransform用の変数を用意
	//-------------------------------------

	Transform uvTransformSprite{
		{1.0f,1.0f,1.f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	//-------------------------------------
	// Instancing用に最大約数分のTransformを用意
	//-------------------------------------
	
	// Δtを定義。とりあえず60fps固定
	const float kDeltaTime = 1.0f / 60.0f;

	// 乱数生成器の初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	Particle particles[kNumMaxInstance];
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		particles[index] = MakeNewParticle(randomEngine);
	}

	//-------------------------------------
	//VertexShaderで利用するtransformationMatrix用のResourceを作成
	//-------------------------------------
	//Sprite用のTransformationMatrix用のリソースを作る
	//Matrix4x4１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformMatrix));
	//データを書き込む
	TransformMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得する
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MyMath::MakeIdentity4x4();
	transformationMatrixDataSprite->World = MyMath::MakeIdentity4x4();
	//CPUで動かす用のTransformを作る
	Transform transformSprite{ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };

	//-------------------------------------
	//WVPMatrixを作って書き込む
	//-------------------------------------

	//Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite = MyMath::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
	Matrix4x4 viewMatrixSprite = MyMath::MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MyMath::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = MyMath::Multiply(worldMatrixSprite, MyMath::Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
	transformationMatrixDataSprite->World = worldMatrixSprite;

	//-------------------------------------
	//WorldViewProjectionMatrixを作る
	//-------------------------------------

	//zが-５の位置でｚ+の方向を向いているカメラ
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{std::numbers::pi_v<float> / 3.0f,std::numbers::pi_v<float>,0.0f},{0.0f,23.0f,-10.0f} };

	//-------------------------------------
	//Textureを読んで転送する
	//-------------------------------------

	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resource/circle.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(metadata);
	dxCommon->UploadTextureData(textureResource, mipImages);

	//-------------------------------------
	//二枚目のTextureを読んで転送する
	//-------------------------------------

	//DirectX::ScratchImage mipImages2 = LoadTexture("resource/monsterBall.png");
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture("resource/uvChecker.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(metadata2);
	dxCommon->UploadTextureData(textureResource2, mipImages2);

	//-------------------------------------
	//ShaderResourceViewを作る
	//-------------------------------------

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(),dxCommon->GetDescriptorSizeSRV(),1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 1);

	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	//-------------------------------------
	//二枚目のShaderResourceViewを作る
	//-------------------------------------

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//２Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	//index = 2の位置にDescriptorを作成する
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);

	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	//-------------------------------------
	// 3つ目
	//-------------------------------------

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	// 構造体のレイアウトは自由に設定できるため不明であるからこの設定
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// TextureではなくBufferとしての利用なので
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	// 当面0にしておく
	instancingSrvDesc.Buffer.FirstElement = 0;
	// この設定で固定
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	// StructuredBufferとしてアクセスする要素数。今回は10個すべてにアクセスするのでkNumInstanc
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	// StructuredBufferで利用する構造体のサイズ
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 3);

	dxCommon->GetDevice()->CreateShaderResourceView(instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	bool isUpdate = false;

	while (true) {
		//Windowのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

		//-------------------------------------
		//入力処理の更新
		//-------------------------------------

		input->Update();

		//-------------------------------------
		//CBufferの中身を更新する
		//-------------------------------------
		//transform.rotate.y += 0.03f;
		Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

		//-------------------------------------
		//WVPMatrixを作成して設定する
		//-------------------------------------
		Matrix4x4 cameraMatrix = MyMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = MyMath::Inverse(cameraMatrix);
		//透視投影行列を計算
		Matrix4x4 projectionMatrix = MyMath::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.f);
		Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;

		//-------------------------------------
		//UVTransform用の行列を作成する
		//-------------------------------------

		Matrix4x4 uvTransformMatrix = MyMath::MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = MyMath::Multiply(uvTransformMatrix, MyMath::MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = MyMath::Multiply(uvTransformMatrix, MyMath::MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;

		//-------------------------------------
		// 
		//-------------------------------------

		// 描画すべきインスタンス数
		uint32_t numInstance = 0;
	
		if (isUpdate) {

			for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
				// 生存時間を過ぎていたら更新せず描画対象にしない
				if (particles[index].lifeTime <= particles[index].currentTime) {
					continue;
				}

				Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(particles[index].transform.scale, particles[index].transform.rotate, particles[index].transform.translate);
				Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
				instancingData[index].WVP = worldViewProjectionMatrix;
				instancingData[index].World = worldMatrix;
				instancingData[index].color = particles[index].color;


				particles[index].transform.translate += particles[index].velocity * kDeltaTime;
				// 経過時間を足す
				particles[index].currentTime += kDeltaTime;
				// 徐々に消す
				float alpha = 1.0f - (particles[index].currentTime / particles[index].lifeTime);
				// α値に適用
				instancingData[numInstance].color.w = alpha;

				// 生きているParticleの数を1つカウントする
				++numInstance;
			}
		} else {
			for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
				// 生存時間を過ぎていたら更新せず描画対象にしない
				if (particles[index].lifeTime <= particles[index].currentTime) {
					continue;
				}

				Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(particles[index].transform.scale, particles[index].transform.rotate, particles[index].transform.translate);
				Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
				instancingData[index].WVP = worldViewProjectionMatrix;
				instancingData[index].World = worldMatrix;
				instancingData[index].color = particles[index].color;
				// 生きているParticleの数を1つカウントする
				++numInstance;
			}

		// 裏が見えているので反対側に回す回転行列を作成
		Matrix4x4 backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);
		// 常にカメラのの方を向く板ポリの行列（BillboardMatrix）を作成
		Matrix4x4 billboardMatrix = MyMath::Multiply(backToFrontMatrix, cameraMatrix);
		// 平行移動成分はいらない
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;

		for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
			// 生存時間を過ぎていたら更新せず描画対象にしない
			if (particles[index].lifeTime <= particles[index].currentTime) {
				continue;
			}
			
			Matrix4x4 scaleMatrix = MyMath::MakeScaleMatrix(particles[index].transform.scale);

			Matrix4x4 rotateXMatrix = MyMath::MakeRotateXMatrix(particles[index].transform.rotate.x);
			Matrix4x4 rotateZMatrix = MyMath::MakeRotateZMatrix(particles[index].transform.rotate.z);
			Matrix4x4 rotateXYZMatrix = MyMath::Multiply(rotateXMatrix, MyMath::Multiply(billboardMatrix, rotateZMatrix));

			Matrix4x4 translateMatrix = MyMath::MakeTranslateMatrix(particles[index].transform.translate);

			Matrix4x4 worldMatrix = MyMath::Multiply(scaleMatrix, MyMath::Multiply(rotateXYZMatrix, translateMatrix));

			Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
			instancingData[index].WVP = worldViewProjectionMatrix;
			instancingData[index].World = worldMatrix;
			instancingData[index].color = particles[index].color;

			

			particles[index].transform.translate += particles[index].velocity * kDeltaTime;
			// 経過時間を足す
			particles[index].currentTime += kDeltaTime;
			instancingData[numInstance].WVP = worldViewProjectionMatrix;
			instancingData[numInstance].World = worldMatrix;
			instancingData[numInstance].color = particles[index].color;
			
			// 徐々に消す
			float alpha = 1.0f - (particles[index].currentTime / particles[index].lifeTime);
			// α値に適用
			instancingData[numInstance].color.w = alpha;

			// 生きているParticleの数を1つカウントする
			++numInstance;
		
		}

		//-------------------------------------
		//フレームの始まる旨を告げる
		//-------------------------------------
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//-------------------------------------
		//ゲームの更新処理でパラメータを変更したいタイミングでImGuiの処理を行う
		//-------------------------------------
		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		//ImGui::ShowDemoWindow();
		ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.05f);
		ImGui::SliderAngle("CameraRotateX", &cameraTransform.rotate.x);
		ImGui::SliderAngle("CameraRotateY", &cameraTransform.rotate.y);
		ImGui::SliderAngle("CameraRotateZ", &cameraTransform.rotate.z);

		ImGui::Checkbox("UpdateParticle", &isUpdate);

		ImGui::SliderAngle("SphereTranslateX", &transform.translate.x);
		ImGui::SliderAngle("SphereTranslateY", &transform.translate.y);
		ImGui::SliderAngle("SphereTranslateZ", &transform.translate.z);

		ImGui::SliderAngle("SphereRotateX", &transform.rotate.x);
		ImGui::SliderAngle("SphereRotateY", &transform.rotate.y);
		ImGui::SliderAngle("SphereRotateZ", &transform.rotate.z);

		const char* items[] = { "None","Normal","kBlendModeAdd","kBlendModeSubtract","kBlendModeMultily","kBlendModeScreen" };
		static int item_current = static_cast<int>(blendMode);
		if (ImGui::Combo("Blend", &item_current, items, IM_ARRAYSIZE(items))) {
			blendMode = static_cast<BlendMode>(item_current);
		}

		//-------------------------------------
		//ブレンドモードが変更された場合、パイプラインステートを再作成
		//-------------------------------------

		static BlendMode previousBlendMode = blendMode;
		if (previousBlendMode != blendMode) {
			SetBlendState(blendDesc, blendMode);
			graphicsPipelineStateDesc.BlendState = blendDesc;
			graphicsPipelineState = CreateGraphicsPipelineState(dxCommon->GetDevice(), graphicsPipelineStateDesc);
			previousBlendMode = blendMode;
		}
	
		//-------------------------------------
		// 描画前処理
		//-------------------------------------

		dxCommon->PreDraw();

		//-------------------------------------
		//ゲームの処理が終わり描画処理に入る前に、ImGuiの内部コマンドを生成する
		//-------------------------------------

		ImGui::Render();

		//-------------------------------------
		//コマンドを積んで描画
		//-------------------------------------		

		//RootSignatureを設定。PSOに設定してるけど別途設定が必要
		dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());//PSOを設定
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
		//形状を設定。PSOに設定してるものとはまた別。同じものを設定すると考えておけばいい
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//マテリアルCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, wvpResource->GetGPUVirtualAddress());

		//wvp用のCBufferの場所を設定
		//RootParameter[1]に対してCBVの設定
		
		// Instancing用のDataを読むためにStructuredBufferのSRVを設定する
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);

		//SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である
		//変数を見て利用するSRVを決める。チェックがはいいているとき（=true）のとき、モンスターボールを使い、falseのときはuvCheckerを使う
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

		//描画！（DrawCall/ドローコール）。kNumInstanc(今回は10)だけInstance描画を行う
		dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), numInstance, 0, 0);

		//-------------------------------------
		// 画面表示できるようにする
		//-------------------------------------

		//実際のcommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		//-------------------------------------
		// 描画後処理
		//-------------------------------------

		dxCommon->PostDraw();

	}

	//-------------------------------------
	//COMの終了処理
	//-------------------------------------

	CoUninitialize();

	//-------------------------------------
	//ImGuiの終了処理
	//-------------------------------------
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//-------------------------------------
	//WindowsAPIの終了処理
	//-------------------------------------

	winApp->Finalize();

	//-------------------------------------
	//解放処理
	//-------------------------------------

	/*CloseHandle(fenceEvent);
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();*/
	delete input;
	delete winApp;
	delete dxCommon;

#ifdef _DEBUG
	//debugController->Release();
#endif 

	return 0;
}