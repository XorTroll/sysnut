#include "install/remote_nsp.hpp"

#include "util/title_util.hpp"
#include "error.hpp"
#include "debug.h"

namespace tin::install::nsp
{
    RemoteNSP::RemoteNSP(std::string url) :
        m_download(url)
    {

    }

    // TODO: Do verification: PFS0 magic, sizes not zero
    void RemoteNSP::RetrieveHeader()
    {
        printf("Retrieving remote NSP header...\n");

        // Retrieve the base header
        m_headerBytes.resize(sizeof(PFS0BaseHeader), 0);
        m_download.BufferDataRange(m_headerBytes.data(), 0x0, sizeof(PFS0BaseHeader), nullptr);

        // Retrieve the full header
        size_t remainingHeaderSize = this->GetBaseHeader()->numFiles * sizeof(PFS0FileEntry) + this->GetBaseHeader()->stringTableSize;
        m_headerBytes.resize(sizeof(PFS0BaseHeader) + remainingHeaderSize, 0);
        m_download.BufferDataRange(m_headerBytes.data() + sizeof(PFS0BaseHeader), sizeof(PFS0BaseHeader), remainingHeaderSize, nullptr);

        LOG_DEBUG("Full header: \n");
    }

    void RemoteNSP::RetrieveAndProcessNCA(NcmNcaId ncaId, std::function<void (void* blockBuf, size_t bufSize, size_t blockStartOffset, size_t ncaSize)> processBlockFunc, std::function<void (size_t sizeRead)> progressFunc)
    {
        const PFS0FileEntry* fileEntry = this->GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = this->GetFileEntryName(fileEntry);

        LOG_DEBUG("Retrieving %s\n", ncaFileName.c_str());

        size_t ncaSize = fileEntry->fileSize;
        u64 fileOff = 0;
        size_t readSize = 0x200000; // 8MB buff
        auto readBuffer = std::make_unique<u8[]>(readSize);

        if (readBuffer == NULL)
			printf("Failed to allocate read buffer for %s\n", ncaFileName.c_str());
            return;

        while (fileOff < ncaSize) 
        {   
            if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;

            m_download.BufferDataRange(readBuffer.get(), this->GetDataOffset() + fileEntry->dataOffset + fileOff, readSize, progressFunc);

            if (processBlockFunc != nullptr)
                processBlockFunc(readBuffer.get(), readSize, fileOff, ncaSize);

            fileOff += readSize;
        }
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntry(unsigned int index)
    {
        if (index >= this->GetBaseHeader()->numFiles)
            THROW_FORMAT("File entry index is out of bounds\n")
    
        size_t fileEntryOffset = sizeof(PFS0BaseHeader) + index * sizeof(PFS0FileEntry);

        if (m_headerBytes.size() < fileEntryOffset + sizeof(PFS0FileEntry))
            THROW_FORMAT("Header bytes is too small to get file entry!");

        return reinterpret_cast<PFS0FileEntry*>(m_headerBytes.data() + fileEntryOffset);
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByExtension(std::string extension)
    {
        for (unsigned int i = 0; i < this->GetBaseHeader()->numFiles; i++)
        {
            const PFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string name(this->GetFileEntryName(fileEntry));
            auto foundExtension = name.substr(name.find(".") + 1); 

            if (foundExtension == extension)
                return fileEntry;
        }

        return nullptr;
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByName(std::string name)
    {
        for (unsigned int i = 0; i < this->GetBaseHeader()->numFiles; i++)
        {
            const PFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string foundName(this->GetFileEntryName(fileEntry));

            if (foundName == name)
                return fileEntry;
        }

        return nullptr;
    }

    const PFS0FileEntry* RemoteNSP::GetFileEntryByNcaId(const NcmNcaId& ncaId)
    {
        const PFS0FileEntry* fileEntry = nullptr;
        std::string ncaIdStr = tin::util::GetNcaIdString(ncaId);

        if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".nca")) == nullptr)
        {
            if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.nca")) == nullptr)
            {
                return nullptr;
            }
        }

        return fileEntry;
    }

    const char* RemoteNSP::GetFileEntryName(const PFS0FileEntry* fileEntry)
    {
        u64 stringTableStart = sizeof(PFS0BaseHeader) + this->GetBaseHeader()->numFiles * sizeof(PFS0FileEntry);
        return reinterpret_cast<const char*>(m_headerBytes.data() + stringTableStart + fileEntry->stringTableOffset);
    }

    const PFS0BaseHeader* RemoteNSP::GetBaseHeader()
    {
        if (m_headerBytes.empty())
            THROW_FORMAT("Cannot retrieve header as header bytes are empty. Have you retrieved it yet?\n");

        return reinterpret_cast<PFS0BaseHeader*>(m_headerBytes.data());
    }

    u64 RemoteNSP::GetDataOffset()
    {
        if (m_headerBytes.empty())
            THROW_FORMAT("Cannot get data offset as header is empty. Have you retrieved it yet?\n");

        return m_headerBytes.size();
    }
}