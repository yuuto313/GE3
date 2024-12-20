#include "Sprite.h"
#include "SpriteCommon.h"
#include "WinApp.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "GameMath.h"

Sprite::Sprite()
{
}

Sprite::~Sprite()
{
}

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	// 引数で受け取ってメンバ変数に記録する
	this->pSpriteCommon_ = spriteCommon;
	this->textureFilePath_ = textureFilePath;

	//-------------------------------------
	// Transform情報を作る
	//-------------------------------------

	transform_.Initilaize();
	transform_.scale_ = { size_.x,size_.y,1.0f };
	transform_.rotate_ = { 0.0f,0.0f,rotation_ };
	transform_.translate_ = { position_.x,position_.y,0.0f };

	uvTransform_.Initilaize();


	//-------------------------------------
	// 頂点データ作成
	//-------------------------------------

	CreateVertexData();

	//-------------------------------------
	// 配列データ作成
	//-------------------------------------

	CreateIndexData();

	//-------------------------------------
	// マテリアルデータ作成
	//-------------------------------------
	
	CreateMaterialData();

	//-------------------------------------
	// 座標変換行列データ作成
	//-------------------------------------
	
	CreateTrasnformationMatrixData();

	//-------------------------------------
	// テクスチャ番号の検索と記録
	//-------------------------------------

	textureIndex_ = TextureManager::GetInstance()->GetSrvIndex(textureFilePath_);

	AdjustTextureSize();

}

void Sprite::Update()
{
	//-------------------------------------
	// 頂点リソースにデータを書き込む
	//-------------------------------------

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	// テクスチャ範囲指定
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

	// アンカーポイント反映処理
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	// 左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}

	// 上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// １枚目の三角形
	// 左下
	vertexData_[0].position = { left,bottom,0.0f,1.0f };
	vertexData_[0].texcoord = { tex_left,tex_bottom };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };
	// 左上
	vertexData_[1].position = { left,top,0.0f,1.0f };
	vertexData_[1].texcoord = { tex_left,tex_top };
	vertexData_[1].normal = { 0.0f,0.0f,-1.0f };

	// 右下
	vertexData_[2].position = { right,bottom,0.0f,1.0f };
	vertexData_[2].texcoord = { tex_right,tex_bottom };
	vertexData_[2].normal = { 0.0f,0.0f,-1.0f };

	// 右上
	vertexData_[3].position = { right,top,0.0f,1.0f };
	vertexData_[3].texcoord = { tex_right,tex_top };
	vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

	//-------------------------------------
	// インデックスリソースにデータを書き込む
	//-------------------------------------

	// 左下
	indexData_[0] = 0;
	// 左上
	indexData_[1] = 1;
	// 右下
	indexData_[2] = 2;
	// 左上
	indexData_[3] = 1;
	// 右上
	indexData_[4] = 3;
	// 右下
	indexData_[5] = 2;

	//-------------------------------------
	// Transform情報を作る
	//-------------------------------------

	CreateWVPMatrix();
}

void Sprite::ImGui()
{
#ifdef _DEBUG
	ImGui::Begin("BlendDesc");
	ImGui::ColorEdit4("Color", &materialData_->color.x);
	ImGui::End();
#endif // _DEBUG
	
}

void Sprite::Draw()
{
	//-------------------------------------
	// VettexBufferViewを設定
	//-------------------------------------

	// Spriteの描画。変更が必要なものだけ変更する
	// VBVを設定する
	pSpriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	
	//-------------------------------------
	// IndexBufferViewを設定
	//-------------------------------------
	
	// IBVを設定
	pSpriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

	//-------------------------------------
	// マテリアルCBufferの場所を設定
	//-------------------------------------

	//マテリアルCBufferの場所を設定
	pSpriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	//-------------------------------------
	// 座標変換行列CBufferの場所を設定
	//-------------------------------------
	
	// wvp用のCBufferの場所を設定
	// RootParameter[1]に対してCBVの設定
	pSpriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	//-------------------------------------
	// SRVのDescriptorTableの先頭を設定
	//-------------------------------------

	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, textureIndex_);

	//-------------------------------------
	// 描画!(DrawCall/ドローコール)
	//-------------------------------------

	pSpriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::CreateVertexData()
{
	//-------------------------------------
	// VertexResourceを作る
	//-------------------------------------

	vertexResource_ = pSpriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);

	//-------------------------------------
	// VertexBufferViewを作成する(値を設定するだけ)
	//-------------------------------------

	// リソースの先頭アドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点６つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// １頂点分のサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//-------------------------------------
	// VertexResourceにデータを書き込むためのアドレスを取得してVertexDataに割り当てる
	//-------------------------------------

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

}

void Sprite::CreateIndexData()
{
	//-------------------------------------
	// IndexResourceを作る
	//-------------------------------------

	indexResource_ = pSpriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);

	//-------------------------------------
	// IndexBufferViewを作成する(値を設定するだけ)
	//-------------------------------------

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//-------------------------------------
	// IndexResourceにデータを書き込むためのアドレスを取得してIndexDataに割り当てる
	//-------------------------------------

	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

}

void Sprite::CreateMaterialData()
{
	//-------------------------------------
	// マテリアルリソースを作る
	//-------------------------------------

	materialResource_ = pSpriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));

	//-------------------------------------
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	//-------------------------------------

	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	//-------------------------------------
	// マテリアルデータの初期値を書き込む
	//-------------------------------------

	// 今回は白を書き込み
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// SpriteはLightingしないのでfalseを設定する
	materialData_->enableLighting = false;
	// 単位行列で初期化
	materialData_->uvTransform = GameMath::MakeIdentity4x4();

}

void Sprite::CreateTrasnformationMatrixData()
{
	//-------------------------------------
	// 座標変換行列リソースを作る
	//-------------------------------------

	transformationMatrixResource_ = pSpriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	
	//-------------------------------------
	// 座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
	//-------------------------------------

	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	//-------------------------------------
	// 単位行列を書き込んでおく
	//-------------------------------------

	transformationMatrixData_->WVP = GameMath::MakeIdentity4x4();
	transformationMatrixData_->World = GameMath::MakeIdentity4x4();

}

void Sprite::CreateWVPMatrix()
{
	transform_.scale_ = { size_.x,size_.y,1.0f };
	transform_.rotate_ = { 0.0f,0.0f,rotation_ };
	transform_.translate_ = { position_.x,position_.y,0.0f };

	//-------------------------------------
	// TransformからWorldMatrixを作る
	//-------------------------------------

	Matrix4x4 worldMatrix = GameMath::MakeAffineMatrix(transform_.scale_, transform_.rotate_, transform_.translate_);

	//-------------------------------------
	// ViewMatrixを作って単位行列を代入
	//-------------------------------------

	Matrix4x4 viewMatrix = GameMath::MakeIdentity4x4();

	//-------------------------------------
	// ProjectionMatrixを作って並行投影行列を書き込む
	//-------------------------------------

	Matrix4x4 projectionMatrix = GameMath::MakeOrthographicMatrix(0.0f,0.0f, float(WinApp::kClientWidth) ,float(WinApp::kClientHeight), 0.0f, 100.f);

	Matrix4x4 wvpMatrix = GameMath::Multiply(worldMatrix, GameMath::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData_->WVP = wvpMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Sprite::AdjustTextureSize()
{
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}
