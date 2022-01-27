#include "Global.h"

RelocAddr <_AddItem> AddItem_Internal(0x4279D0);

void AddItem(TESObjectREFR* refr, TESForm* item, UInt32 count, bool isSilent) {
	if (!refr || !item)
		return;

	if (count == 0)
		return;

	AddItemData addItemData = { refr, 0, isSilent };
	ItemData itemData = { 0 };
	itemData.item = item;
	itemData.count = count;
	itemData.unk28 = 1.0f;

	AddItem_Internal(&addItemData, &itemData);
}

const tArray<Actor::MiddleProcess::Data08::EquipData>* GetEquipDataArray() {
	if (!*g_player || !(*g_player)->middleProcess || !(*g_player)->middleProcess->unk08)
		return nullptr;

	tArray<Actor::MiddleProcess::Data08::EquipData> equipDataArray = reinterpret_cast<tArray<Actor::MiddleProcess::Data08::EquipData> &>((*g_player)->middleProcess->unk08->equipData);
	if (equipDataArray.count == 0)
		return nullptr;

	return &reinterpret_cast<tArray<Actor::MiddleProcess::Data08::EquipData> &>((*g_player)->middleProcess->unk08->equipData);
}

Actor::MiddleProcess::Data08::EquipData* GetEquipDataByFormID(UInt32 formId) {
	const tArray<Actor::MiddleProcess::Data08::EquipData>* equipDataArray = GetEquipDataArray();
	if (!equipDataArray)
		return nullptr;

	for (UInt32 ii = 0; ii < equipDataArray->count; ii++) {
		if (equipDataArray->entries[ii].item->formID == formId)
			return &equipDataArray->entries[ii];
	}

	return nullptr;
}

Actor::MiddleProcess::Data08::EquipData* GetEquipDataByEquipIndex(EquipIndex equipIndex) {
	const tArray<Actor::MiddleProcess::Data08::EquipData>* equipDataArray = GetEquipDataArray();
	if (!equipDataArray)
		return nullptr;

	for (UInt32 ii = 0; ii < equipDataArray->count; ii++) {
		UInt32 eIdx = static_cast<UInt32>(equipDataArray->entries[ii].unk18);
		if (eIdx == equipIndex)
			return &equipDataArray->entries[ii];
	}

	return nullptr;
}

UInt16 GetCurrentAmmoCapacity() {
	const tArray<Actor::MiddleProcess::Data08::EquipData>* equipDataArray = GetEquipDataArray();
	if (!equipDataArray) 
		return 0;

	for (UInt32 ii = 0; ii < equipDataArray->count; ii++) {
		if (!IsThrowableWeapon(equipDataArray->entries[ii].unk18))
			return GetCurrentAmmoCapacity(equipDataArray->entries[ii].item, (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(equipDataArray->entries[ii].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData));
	}

	return 0;
}

UInt16 GetCurrentAmmoCapacity(TESForm* weap, TESObjectWEAP::InstanceData* weapInst) {
	if (!weapInst) {
		if (!weap)
			return 0;

		TESObjectWEAP* objWeap = DYNAMIC_CAST(weap, TESForm, TESObjectWEAP);
		if (!objWeap)
			return 0;
		weapInst = &objWeap->weapData;
	}
	return weapInst->ammoCapacity;
}

UInt32 GetInventoryItemCount(Actor* actor, TESForm* item) {
	if (!actor || !item)
		return 0;

	BGSInventoryList* inventory = actor->inventoryList;
	if (!inventory)
		return 0;

	UInt32 totalItemCount = 0;
	inventory->inventoryLock.LockForRead();
	for (UInt32 ii = 0; ii < inventory->items.count; ii++) {
		if (inventory->items[ii].form == item) {
			BGSInventoryItem::Stack* sp = inventory->items[ii].stack;
			while (sp) {
				totalItemCount += sp->count;
				sp = sp->next;
			}
			break;
		}
	}
	inventory->inventoryLock.Unlock();

	return totalItemCount;
}

bool IsThrowableWeapon(UInt32 equipIndex) {
	return equipIndex == EquipIndex::kEquipIndex_Throwable;
}

static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

template <typename T>
T GetOffset(const void* baseObject, int offset) {
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

TESForm* GetFormFromIdentifier(const std::string& pluginName, const std::string& formIdStr) {
	UInt32 formID = std::stoul(formIdStr, nullptr, 16) & 0xFFFFFF;
	return GetFormFromIdentifier(pluginName, formID);
}

TESForm* GetFormFromIdentifier(const std::string& pluginName, const UInt32 formId) {
	if (!*g_dataHandler)
		return nullptr;

	const ModInfo* mod = (*g_dataHandler)->LookupModByName(pluginName.c_str());
	if (!mod || mod->modIndex == -1)
		return nullptr;

	UInt32 actualFormId = formId;
	UInt32 flags = GetOffset<UInt32>(mod, 0x334);
	if (flags & (1 << 9)) {
		actualFormId &= 0xFFF;
		actualFormId |= 0xFE << 24;
		actualFormId |= GetOffset<UInt16>(mod, 0x372) << 12;
	}
	else {
		actualFormId |= (mod->modIndex) << 24;
	}
	return LookupFormByID(actualFormId);
}