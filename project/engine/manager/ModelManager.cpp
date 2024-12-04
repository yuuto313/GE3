#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ModelManager;
	}

	return instance;
}

void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	pModelCommon_ = new ModelCommon();
	pModelCommon_->Initialize(dxCommon);
}

void ModelManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みのモデルを検索
	if (models_.contains(filePath)) {
		// 読み込み済みなら早期return
		return;
	}

	//-------------------------------------
	// モデルの生成とファイル読み込み、初期化
	//-------------------------------------

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(pModelCommon_, "resource", filePath);

	// モデルをmapコンテナに格納する
	models_.insert(std::make_pair(filePath, std::move(model)));

}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		// 読み込みモデルを戻り値としてreturn
		return models_.at(filePath).get();
	}

	return nullptr;
}
