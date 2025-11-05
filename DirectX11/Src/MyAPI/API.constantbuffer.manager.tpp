
// ※ このファイルは公開インターフェース用のヘッダーファイルです
// 　 利用者によるファイル内の実装変更を想定していないので直接行わないでください

//============================================================================
// 
// 定数バッファマネージャー、テンプレート実装ファイル [constantbuffer.manager.tpp]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//============================================================================
// 定数バッファの更新
//============================================================================
template <typename T>
void CConstantBufferManager::UpdateConstantBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstantBuffer, const T& Data)
{
	// MapしたGPUメモリのポインタ格納先
	D3D11_MAPPED_SUBRESOURCE MappedBuff = {};

	// GPUの定数バッファをCPU側がいじれるようにロック
	pContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedBuff);

	// CPU側で用意した構造体データをGPUに渡す
	auto* pConstantBuff = reinterpret_cast<T*>(MappedBuff.pData);
	*pConstantBuff = Data;

	// 書き込みが完了したのでアンロック
	pContext->Unmap(pConstantBuffer, 0);
}