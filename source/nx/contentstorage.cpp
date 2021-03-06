#include "nx/ipc/ncm_ext.h"
#include "nx/contentstorage.h"
#include "log.h"


ContentStorage::ContentStorage(FsStorageId _storageId) 
{
	storageId() = _storageId;
#ifndef _MSC_VER
	if (ncmOpenContentStorage(_storageId, &m_contentStorage))
	{
		error("Failed to open NCM ContentStorage %d\n", storageId());
	}
	else
	{
		print("opened content sotrage!");
	}
#endif
}

ContentStorage::~ContentStorage()
{
#ifndef _MSC_VER
	print("closing content sotrage!");
    serviceClose(&m_contentStorage.s);
#endif
}

bool ContentStorage::createPlaceholder(const NcaId& placeholderId, const NcaId& registeredId, size_t size)
{
#ifndef _MSC_VER
	if (ncmCreatePlaceHolder(&m_contentStorage, &placeholderId, &registeredId, size))
	{
		error("Failed to create placeholder\n");
		return false;
	}
#endif
	return true;
}
            
bool ContentStorage::deletePlaceholder(const NcaId &placeholderId)
{
#ifndef _MSC_VER
	if (ncmDeletePlaceHolder(&m_contentStorage, &placeholderId))
	{
		error("Failed to delete placeholder\n");
		return false;
	}
#else
	string ncaFile = hx(placeholderId) + ".nca";
	unlink(ncaFile.c_str());
#endif
	return true;
}

bool ContentStorage::writePlaceholder(const NcaId &placeholderId, u64 offset, void *buffer, size_t bufSize)
{
#ifndef _MSC_VER
	if (ncmWritePlaceHolder(&m_contentStorage, &placeholderId, offset, buffer, bufSize))
	{
		error("Failed to write to placeholder\n");
		return false;
	}
#else
	string ncaFile = hx(placeholderId) + ".nca";
	FILE* f = fopen(ncaFile.c_str(), "ab");
	fseek(f, offset, 0);
	fwrite(buffer, 1, bufSize, f);
	fclose(f);
#endif
	return true;
}

bool ContentStorage::reg(const NcaId &placeholderId, const NcaId &registeredId)
{
#ifndef _MSC_VER
	if (ncmContentStorageRegister(&m_contentStorage, (const NcmNcaId*)&registeredId, (const NcmNcaId*)&placeholderId))
	{
		error("Failed to register placeholder NCA\n");
		return false;
	}
#endif
	return true;
}

bool ContentStorage::del(const NcaId &registeredId)
{
#ifndef _MSC_VER
	if (ncmDelete(&m_contentStorage, &registeredId))
	{
		error("Failed to delete registered NCA\n");
		return false;
	}
#endif
	return true;
}

bool ContentStorage::has(const NcaId &registeredId)
{
#ifndef _MSC_VER
	bool hasNCA = false;
	if (ncmContentStorageHas(&m_contentStorage, (const NcmNcaId*)&registeredId, &hasNCA))
	{
		error("Failed to check if NCA is present");
	}
	return hasNCA;
#else
	return false;
#endif
}

string ContentStorage::getPath(const NcaId& registeredId)
{
#ifndef _MSC_VER
	string result;
	result.resize(FS_MAX_PATH);
	if (ncmContentStorageGetPath(&m_contentStorage, (const NcmNcaId*)&registeredId, (char*)result.buffer(), FS_MAX_PATH))
	{
		error("Failed to get installed NCA path");
	}
    return result;
#else
	return hx(registeredId) + ".nca";
#endif
}
