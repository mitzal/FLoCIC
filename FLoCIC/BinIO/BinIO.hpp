#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <type_traits>
#include <span>
#include <optional>

namespace BinIO {

	struct IWriter {
		virtual void OpenBinaryStream(const std::string& path) = 0;
		virtual void CloseStream() = 0;
		virtual void Write(const uint8_t* buffer, uint32_t size) = 0;
	};
	
	struct IReader {
		virtual void OpenBinaryStream(const std::string& path) = 0;
		virtual void CloseStream() = 0;
		virtual uint32_t Read(uint8_t* buffer, uint32_t bufferSize) = 0;
		virtual void ResetToStart() = 0;
	};

	struct StdWriter : public IWriter {
		void OpenBinaryStream(const std::string& path) override
		{
			m_OutputStream.open(path, std::ios::binary);
			if (!m_OutputStream.is_open()) {
				throw std::runtime_error("Could not open OutputStream");
			}
		}

		void CloseStream() override
		{
			m_OutputStream.close();
		}

		void Write(const uint8_t* buffer, uint32_t size) override
		{
			m_OutputStream.write(reinterpret_cast<const char*>(buffer), size);
			if (!m_OutputStream.good()) { 
				throw std::runtime_error("Error when flushing data to a file"); 
			}
		}
	private:
		std::ofstream m_OutputStream;
	};

	struct StdReader : public IReader {
		void OpenBinaryStream(const std::string& path) override
		{
			m_InputStream.open(path, std::ios::binary);
			if (!m_InputStream.is_open() || !m_InputStream.good()) {
				throw std::runtime_error("Could not open InputStream");
			}
		}
		void CloseStream() override
		{
			m_InputStream.close();
		}
		uint32_t Read(uint8_t* buffer, uint32_t bufferSize) override
		{
			m_InputStream.read(reinterpret_cast<char*>(buffer), bufferSize);
			if (m_InputStream.bad()) { throw std::runtime_error("Stream reading operation failed"); }
			return static_cast<uint32_t>(m_InputStream.gcount());
		}
		void ResetToStart() override
		{
			m_InputStream.clear();
			m_InputStream.seekg(0);
		}

	private:
		std::ifstream m_InputStream;
	};

	struct MemoryWriter : IWriter {
		void OpenBinaryStream(const std::string& path) override
		{
			m_StreamOpened = true;
		}
		
		void CloseStream() override
		{
			m_StreamOpened = false;
		}

		void Write(const uint8_t* buffer, uint32_t size) override
		{
			m_Buffer.insert(m_Buffer.end(), buffer, buffer + size);
		}

		const std::vector<uint8_t>& GetBuffer() const
		{
			if (m_StreamOpened) { throw std::runtime_error("MemoryStream is still opened, cannot get buffer"); }
			return m_Buffer;
		}
	
	private:
		std::vector<uint8_t> m_Buffer;
		bool m_StreamOpened = false;
	};

	struct MemoryReader : IReader {
		MemoryReader(const std::span< const uint8_t>& data)
		{
			m_Data = data;
		}
		void OpenBinaryStream(const std::string& path) override
		{
			ResetToStart();
		}
		void CloseStream() override
		{
			ResetToStart();
		}
		uint32_t Read(uint8_t* buffer, uint32_t bufferSize) override
		{
			uint32_t sz = std::min(uint32_t(m_Data.size() - m_Index), bufferSize);
			if (sz > 0) {
				if (memcpy_s(buffer, bufferSize, m_Data.data() + m_Index, sz) != 0) {
					throw std::runtime_error("Error when copying data from memory buffer");
				}
				m_Index += sz;
			}
			return sz;
		}
		void ResetToStart() override
		{
			m_Index = 0;
		}
	private:
		std::span<const uint8_t> m_Data;
		size_t m_Index = 0;
	};

	class BinWriter
	{
	public:
		BinWriter() { } 
		
		BinWriter(const std::shared_ptr<IWriter>& writer, uint32_t bufferSize = 4096) 
		{
			Initialize(writer, bufferSize);
		}

		BinWriter(const std::string& path, uint32_t bufferSize = 4096)
		{
			auto writer = std::make_shared<StdWriter>();
			Initialize(writer, bufferSize, path);
		}

		BinWriter(const BinWriter& second) = default;
		BinWriter(BinWriter&& second) = default;
		BinWriter& operator=(const BinWriter& second) = default;
		BinWriter& operator=(BinWriter&& second) = default;

		~BinWriter()
		{
			Close();
		}

		void Close()
		{
			if (m_Writer != nullptr) {
				if (m_CurrBytePos != 8) {
					IncreaseBufferIndex();
				}

				FlushBuffer();
				m_Writer->CloseStream();
			}
		}

		template<typename T>
		void WriteBit(T b) 
		{
			if (b) {
				m_Buffer[m_BufferIndex] |= 1 << (m_CurrBytePos - 1);
			}
			if (--m_CurrBytePos == 0) { IncreaseBufferIndex(); }
		}

		template<typename T>
		void WriteBits(T value, uint32_t numBits)
		{
			using TT = typename std::remove_cvref<T>::type;
			WriteBitsImpl<TT>(value, numBits);
		}

		template<typename T>
		void Write(T value)
		{
			using TT = typename std::remove_cvref<T>::type;
			WriteBitsImpl<TT>(value, 8u * sizeof(T));
		}


	private:
		void Initialize(const std::shared_ptr<IWriter>& writer, uint32_t bufferSize, const std::string& path = std::string())
		{
			m_BufferSize = bufferSize;
			m_Writer = writer;
			m_Writer->OpenBinaryStream(path);
			m_Buffer.resize(m_BufferSize);
		}

		template<typename T>
		void WriteBitsImpl(T value, uint32_t numBits)
		{
			using U = std::make_unsigned<T>::type;
			U uValue = reinterpret_cast<U&>(value);
			if (numBits < 8 * sizeof(U)) {
				uValue &= (U(1) << numBits) - 1; // set all bits but `numBits` rightmost ones to 0
			}

			if (numBits >= m_CurrBytePos) {
				m_Buffer[m_BufferIndex] |= uValue >> (numBits - m_CurrBytePos);
				numBits -= m_CurrBytePos;
				IncreaseBufferIndex();
			}

			while (numBits >= 8) {
				m_Buffer[m_BufferIndex] = (unsigned char)(uValue >> (numBits - 8));
				numBits -= 8;
				IncreaseBufferIndex();
			}

			if (numBits > 0) {
				m_Buffer[m_BufferIndex] |= (unsigned char)(uValue << (m_CurrBytePos - numBits));
				m_CurrBytePos -= numBits;
				// numBits must be smaller than m_CurrByte pos, therefore IncreaseBufferIndex should never be called here
			}
		}

		void FlushBuffer() 
		{
			if (m_BufferIndex > 0) {
				m_Writer->Write(m_Buffer.data(), m_BufferIndex);
				m_BufferIndex = 0;
			}
		}

		void IncreaseBufferIndex() 
		{
			if (++m_BufferIndex == m_BufferSize) {
				FlushBuffer();
			}
			m_Buffer[m_BufferIndex] = 0;
			m_CurrBytePos = 8;
		}

		std::shared_ptr<IWriter> m_Writer = nullptr;
		std::vector<unsigned char> m_Buffer;
		uint32_t m_BufferIndex = 0;
		uint32_t m_BufferSize = 0;
		uint_fast8_t m_CurrBytePos = 8;
	};

	class BinReader
	{
	public:
		BinReader() {}

		BinReader(const std::shared_ptr<IReader>& reader, uint32_t bufferSize = 4096)
		{
			Initialize(reader, bufferSize);
		}

		BinReader(const std::string& path, uint32_t bufferSize = 4096)
		{
			auto reader = std::make_shared<StdReader>();
			Initialize(reader, bufferSize, path);
		}

		BinReader(const BinReader& second) = default;
		BinReader(BinReader&& second) = default;
		BinReader& operator=(const BinReader& second) = default;
		BinReader& operator=(BinReader&& second) = default;

		~BinReader() 
		{
			Close();
		}

		void Close()
		{
			if (m_Reader != nullptr) {
				m_Reader->CloseStream();
			}
		}

		template<typename T = bool>
		T ReadBit() 
		{
			bool ret = (m_Buffer[m_BufferIndex] & (1 << (m_CurrBytePos - 1))) != 0;
			if (--m_CurrBytePos == 0) { IncreaseBufferIndex(); }
			
			if constexpr (std::is_same<T, bool>::value) { return ret; }
			else { return ret ? T(1) : T(0); }
		}

		template<typename T>
		typename std::remove_cvref<T>::type ReadBits(uint32_t numBits) 
		{
			using TT = std::remove_cvref<T>::type;
			using U = std::make_unsigned<TT>::type;
			
			U value = 0;

			if (numBits >= m_CurrBytePos && m_CurrBytePos < 8) {
				value = ((1 << m_CurrBytePos) - 1) & m_Buffer[m_BufferIndex];
				numBits -= m_CurrBytePos;
				IncreaseBufferIndex();
			}

			while (numBits >= 8) {
				value <<= 8;
				value |= m_Buffer[m_BufferIndex];
				numBits -= 8;
				IncreaseBufferIndex();
			}

			if (numBits > 0) {
				value <<= numBits;
				uint8_t mask = numBits == 8 ? 0xFF : (1 << numBits) - 1;
				value |= (m_Buffer[m_BufferIndex] >> (m_CurrBytePos - numBits)) & mask;
				m_CurrBytePos -= numBits;
			}
			return value;
		}

		template<typename T>
		typename std::remove_cvref<T>::type Read() 
		{
			using TT = std::remove_cvref<T>::type;
			return ReadBits<TT>(8 * sizeof(TT));
		}

		bool Eof() const 
		{ 
			return m_EofBit; 
		}
		
		void ResetToStart() 
		{
			m_Reader->ResetToStart();
			InitReaderImpl();
		}

	private:
		void Initialize(const std::shared_ptr<IReader>& reader, uint32_t bufferSize, const std::string& path = std::string())
		{
			m_Reader = reader;
			m_Reader->OpenBinaryStream(path);

			m_Buffer.resize(bufferSize);
			InitReaderImpl();
		}

		void InitReaderImpl() 
		{
			m_EofBit = false;
			m_BufferIndex = m_BufferSize = static_cast<uint32_t>(m_Buffer.size());
			IncreaseBufferIndex();
		}

		void SetEofBit() 
		{
			m_EofBit = true;
			m_Buffer[0] = '\0';
			m_BufferIndex = 0;
			return;
		}

		void IncreaseBufferIndex() 
		{
			m_CurrBytePos = 8;
			if (m_EofBit) { return; }

			if (++m_BufferIndex >= m_BufferSize) {
				if (static_cast<size_t>(m_BufferSize) < m_Buffer.size()) {
					SetEofBit();
				}

				m_BufferIndex = 0;
				m_BufferSize = m_Reader->Read(m_Buffer.data(), static_cast<uint32_t>(m_Buffer.size()));
				if (m_BufferSize == 0) { SetEofBit(); }
			}
		}

		std::shared_ptr<IReader> m_Reader;
		std::vector<unsigned char> m_Buffer;
		uint32_t m_BufferIndex = 0;
		uint32_t m_BufferSize = 0;
		uint_fast8_t m_CurrBytePos = 8;
		bool m_EofBit = false;
	};
}
