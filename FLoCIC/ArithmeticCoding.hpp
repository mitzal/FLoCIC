#pragma once
#include <string>
#include <vector>
#include <concepts>
#include <ranges>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include "BinIO/BinIO.hpp"

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template<typename T>
concept CountSet = IsAnyOf<T, uint16_t, uint32_t, uint64_t>;

namespace Compression {
	namespace impl {
		template<typename SymT>
		struct SymbolAccessor {
		public:
			SymbolAccessor() = default;
			SymbolAccessor(const std::vector<SymT>& buffer) : m_Buffer(buffer) {}

			SymT GetNextSymbol() {
				if (m_Index < m_Buffer.size()) {
					auto ret = m_Buffer[m_Index];
					m_Index++;
					return ret;
				}
				return SymT(0);
			}
			bool Eof() { return m_Index >= m_Buffer.size(); }
			void ResetToStart() { m_Index = size_t(0); }

		private:
			std::vector<SymT> m_Buffer{};
			size_t m_Index = 0;
		};

		template<typename SymT, typename CountT>
		struct ACe123SymbolCounting {
		public:
			ACe123SymbolCounting() {
				Clear();
			}

			void Clear() {
				// just for the lower bound of the first elt
			
				m_Symbols.resize(1, SymT(0));
				m_Counts.resize(1, CountT(0));
				m_SymbolMap.clear();
			}

			void Initialize(SymbolAccessor<SymT>& reader)
			{
				Clear();

				while (!reader.Eof()) {
					auto sym = reader.GetNextSymbol();
					if (auto idx = GetSymIdx(sym); idx >= 0) { m_Counts[idx]++; }
					else {
						m_SymbolMap.emplace(sym, static_cast<int32_t>(m_Symbols.size()));
						m_Symbols.emplace_back(sym);
						m_Counts.emplace_back(1);
					}
				}

				for (size_t i = 1; i < m_Counts.size(); ++i) {
					m_Counts[i] += m_Counts[i - 1];
				}
			}

			void AddSymAndCount(SymT symbol, CountT high_count) {
				if (high_count < m_Counts.back()) { 
					throw std::runtime_error("Invalid high count added to the SymbolCounter"); 
				}
				if (m_SymbolMap.contains(symbol)) {
					throw std::runtime_error("Symbols duplicated in symbol map");
				}
				m_SymbolMap.emplace(symbol, static_cast<int32_t>(m_Symbols.size()));
				m_Symbols.emplace_back(symbol);
				m_Counts.emplace_back(high_count);
			}

			int32_t GetSymIdx(SymT sym)
			{
				if (auto it = m_SymbolMap.find(sym); it != m_SymbolMap.end()) {
					return it->second;
				}
				return -1;
			}

			CountT GetNumSymbols() const
			{
				return m_Counts.back();
			}

			void AddSymOnIdx(int32_t idx)
			{
				for (size_t i = size_t(idx); i < m_Counts.size(); ++i) {
					m_Counts[i] ++;
				}
			}

			std::tuple<CountT, CountT> GetSymBoundsOnIdx(int32_t idx)
			{
				return std::tuple<CountT, CountT>{m_Counts[idx - 1], m_Counts[idx]};
			}

			std::tuple<CountT, CountT> GetSymBounds(SymT sym)
			{
				auto idx = GetSymIdx(sym);
				if (idx < 1) { throw std::logic_error("Invalid symbol"); }
				return std::tuple<CountT, CountT>{ m_Counts[idx - 1], m_Counts[idx] };
			}

			std::tuple<SymT, CountT, CountT> GetSymFromVal(CountT value)
			{
				auto idx = GetCountIdx(value);
				if (idx < 1) { throw std::logic_error("Invalid value"); }
				return std::tuple<SymT, CountT, CountT>{ m_Symbols[idx], m_Counts[idx - 1], m_Counts[idx] };
			}

			std::span<const CountT> GetCounts() const { return std::span<const CountT>(m_Counts.begin() + 1, m_Counts.end()); }
			std::span<const SymT> GetSymbols() const { return std::span<const SymT>(m_Symbols.begin() + 1, m_Symbols.end()); }

		private:
			int32_t GetCountIdx(CountT count)
			{
				// right-bound is excluded
				if (auto it = std::upper_bound(m_Counts.begin(), m_Counts.end(), count); it != m_Counts.end()) {
					return int32_t(it - m_Counts.begin());
				}
				return -1;
			}

			std::vector<CountT> m_Counts;
			std::vector<SymT> m_Symbols;
			std::unordered_map<SymT, int32_t> m_SymbolMap;
		};
	}

	template<typename SymT, CountSet CountT=uint32_t>
	class ACe123 {
	public: 
		std::vector<uint8_t> Encode(const std::vector<SymT>& data);
		// void Encode(const std::string& inputPath, const std::string& outputPath, const std::vector<SymT>& alphabet = {});

		std::vector<SymT> Decode(const std::vector<uint8_t>& compressedData);
		// void Decode(const std::string& inputPath, const std::string& outputPath);

		std::vector<uint8_t> AdaptiveEncode(const std::vector<SymT>& data, const std::vector<SymT>& alphabet = {});
		// void AdaptiveEncode(const std::string& inputPath, const std::string& outputPath, const std::vector<SymT>& alphabet = {});

		std::vector<SymT> AdaptiveDecode(const std::vector<uint8_t>& compressedData);
		// void AdaptiveDecode(const std::string& inputPath, const std::string& outputPath);
	
		impl::ACe123SymbolCounting<SymT, CountT> DecodeHeader(const std::vector<uint8_t>& compressedData);

	private:
		static_assert(std::is_unsigned<CountT>::value);
		static constexpr CountT g_Max = std::numeric_limits<CountT>::max() >> 1;
		static constexpr CountT g_Half = (g_Max + CountT(1)) / CountT(2);
		static constexpr CountT g_FirstQuarter = g_Half / CountT(2);
		static constexpr CountT g_ThirdQuarter = g_FirstQuarter * CountT(3);

		void InitializeDecodeBuffer();
		void EncodeStepImpl(SymT sym);
		void FinishEncoding();
		SymT DecodeStepImpl();

		void EncodeHeader();
		void DecodeHeader(); 

		void ResetVariables();
		
		CountT m_Low = CountT(0);
		CountT m_High = CountT(0);
		CountT m_Scale = CountT(0);
		CountT m_DecodeBuffer = CountT(0);
		impl::ACe123SymbolCounting<SymT, CountT> m_SymbolCounter;
		BinIO::BinWriter m_BinWriter;
		BinIO::BinReader m_BinReader;
	};

	// impl
	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::EncodeStepImpl(SymT sym)
	{
		auto [sym_low, sym_high] = m_SymbolCounter.GetSymBounds(sym);
		CountT step = (m_High - m_Low + 1) / m_SymbolCounter.GetNumSymbols();
		m_High = m_Low + step * sym_high - 1;
		m_Low = m_Low + step * sym_low;

		while (m_High < g_Half || m_Low >= g_Half) {
			if (m_High < g_Half) {
				m_Low = CountT(2) * m_Low;
				m_High = CountT(2) * m_High + 1;
				
				m_BinWriter.WriteBit(0);
				for (; m_Scale > 0; --m_Scale) {
					m_BinWriter.WriteBit(1);
				}
			}
			else { // m_Low >= g_Half
				m_Low = CountT(2) * (m_Low - g_Half);
				m_High = 2 * (m_High - g_Half) + 1;

				m_BinWriter.WriteBit(1);
				for (; m_Scale > 0; --m_Scale) {
					m_BinWriter.WriteBit(0);
				}
			}
		}
		
		while ((g_FirstQuarter <= m_Low) && (m_High < g_ThirdQuarter)) {
			++m_Scale;
			m_Low = CountT(2) * (m_Low - g_FirstQuarter);
			m_High = CountT(2) * (m_High - g_FirstQuarter) + 1;
		}
	}

	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::FinishEncoding()
	{
		if (m_Low < g_FirstQuarter) {
			m_BinWriter.WriteBit(0u);

			for (CountT i = 0; i < m_Scale + 1; ++i) {
				m_BinWriter.WriteBit(1u);
			}
		}
		else {
			m_BinWriter.WriteBit(1u);
			// BinReader will add missing zeros automatically
		}
	}

	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::InitializeDecodeBuffer()
	{
		m_DecodeBuffer = 0;
		for (CountT i = 1; (i & g_Max) != 0; i <<= 1) {
			m_DecodeBuffer = (m_DecodeBuffer << 1) | m_BinReader.ReadBit<CountT>();
		}
	}

	template<typename SymT, CountSet CountT>
	inline SymT ACe123<SymT, CountT>::DecodeStepImpl()
	{
		CountT step = (m_High - m_Low + CountT(1)) / m_SymbolCounter.GetNumSymbols();
		CountT value = (m_DecodeBuffer - m_Low) / step;

		auto[sym, sym_low, sym_high] = m_SymbolCounter.GetSymFromVal(value);
		
		m_High = m_Low + step * sym_high - 1;
		m_Low = m_Low + step * sym_low;
		while (m_High < g_Half || m_Low >= g_Half) {
			if (m_High < g_Half) {
				m_Low = CountT(2) * m_Low;
				m_High = CountT(2) * m_High + CountT(1);

				m_DecodeBuffer = ((m_DecodeBuffer << 1) | m_BinReader.ReadBit<CountT>()) & g_Max;
			}
			else { //mLow >= g_Half
				m_Low = CountT(2) * (m_Low - g_Half);
				m_High = CountT(2) * (m_High - g_Half) + 1;
				m_DecodeBuffer -= g_Half;
				m_DecodeBuffer = ((m_DecodeBuffer << 1) | m_BinReader.ReadBit<CountT>()) & g_Max;
			}
		}
		
		while ((g_FirstQuarter <= m_Low) && (m_High < g_ThirdQuarter)) {
			m_Low = CountT(2) * (m_Low - g_FirstQuarter);
			m_High = CountT(2) * (m_High - g_FirstQuarter) + CountT(1);

			m_DecodeBuffer -= g_FirstQuarter;
			m_DecodeBuffer = ((m_DecodeBuffer << 1) | m_BinReader.ReadBit<CountT>()) & g_Max;
		}

		return sym;
	}

	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::EncodeHeader()
	{
		// header structure 
		// 2 - bits (version) = 0
		// 1 - bit (0 - non adaptive, 1 - adaptive)
		// In case of non adaptive version
		//		6 bits - symbol count length in bits (SCL) - 1
		//		6 bits - symbol length in bits (SL) - 1
		//		m * (SCL + SL) bits - (m is number of different symbols) - symbol table (symbol, high count)
		//		SCL bits - 0 (end of header)
	
		m_BinWriter.WriteBits(0u, 2);
		m_BinWriter.WriteBit(0u);

		// determine greatest symbol
		const auto& symbols = m_SymbolCounter.GetSymbols();
		const auto& high_counts = m_SymbolCounter.GetCounts();
		if (symbols.empty()) { throw std::runtime_error("Symbol counter not initialized. Cannot generate header for non adaptive version"); }
		
		auto it = std::max_element(symbols.begin(), symbols.end());

		// cast SymT to unsigned if it is not
		uint32_t symWidth = std::bit_width(std::bit_cast<std::make_unsigned<SymT>::type>(*it));
		if constexpr (std::is_signed<SymT>::value) {
			auto min_it = std::min_element(symbols.begin(), symbols.end());
			uint32_t minSymWidth = std::bit_width(std::bit_cast<std::make_unsigned<SymT>::type>(*min_it));
			symWidth = std::max(symWidth, minSymWidth);
		}
		symWidth = std::max(symWidth, uint32_t(1));

		uint32_t countWidth = std::max(uint32_t(std::bit_width(high_counts.back())), uint32_t(1));
		
		m_BinWriter.WriteBits(countWidth - uint32_t(1), 6);
		m_BinWriter.WriteBits(symWidth - uint32_t(1), 6);
		
#ifdef _DEBUG
		if (symbols.size() != high_counts.size()) { throw std::runtime_error("Symbols and high count sizes differ"); }
#endif

		for (size_t i = 0; i < high_counts.size(); ++i) {
			m_BinWriter.WriteBits(high_counts[i], countWidth);
			m_BinWriter.WriteBits(symbols[i], symWidth);
		}
		m_BinWriter.WriteBits(CountT(0), countWidth);
	}

	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::DecodeHeader()
	{
		m_SymbolCounter.Clear();

		// version
		if (m_BinReader.ReadBits<uint8_t>(2) != uint8_t(0)) { throw std::runtime_error("Unsupported version"); }

		// adaptive flag
		bool adaptive = m_BinReader.ReadBit();

		if (adaptive) { throw std::runtime_error("Adaptive mode not supported yet"); }

		// determine greatest symbol
		auto countWidth = m_BinReader.ReadBits<uint32_t>(6) + uint32_t(1);
		auto symWidth = m_BinReader.ReadBits<uint32_t>(6) + uint32_t(1);

		while (true) {
			auto high_count = m_BinReader.ReadBits<CountT>(countWidth);
			if (high_count == 0) { break; }
			auto sym = m_BinReader.ReadBits<SymT>(symWidth);
			
			m_SymbolCounter.AddSymAndCount(sym, high_count);
		}
	}

	template<typename SymT, CountSet CountT>
	inline void ACe123<SymT, CountT>::ResetVariables()
	{
		m_Low = CountT(0);
		m_High = CountT(0);
		m_Scale = CountT(0);
		m_DecodeBuffer = CountT(0);
		m_SymbolCounter = impl::ACe123SymbolCounting<SymT, CountT>{};
		m_BinWriter = BinIO::BinWriter{};
		m_BinReader = BinIO::BinReader{};
	}

	template<typename SymT, CountSet CountT>
	inline std::vector<uint8_t> ACe123<SymT, CountT>::Encode(const std::vector<SymT>& data)
	{
		ResetVariables();
		m_High = g_Max;
		m_Low = m_Scale = CountT(0);

		auto memoryWriter = std::make_shared<BinIO::MemoryWriter>();
		m_BinWriter = BinIO::BinWriter(memoryWriter);

		impl::SymbolAccessor<SymT> accessor(data);
		m_SymbolCounter.Initialize(accessor);
		EncodeHeader();

		accessor.ResetToStart();

		while (!accessor.Eof()) {
			auto sym = accessor.GetNextSymbol();
			EncodeStepImpl(sym);
			auto [sym_low, sym_high] = m_SymbolCounter.GetSymBounds(sym);
		}
		FinishEncoding();

		m_BinWriter.Close();
		return memoryWriter->GetBuffer();
	}

	template<typename SymT, CountSet CountT>
	inline std::vector<SymT> ACe123<SymT, CountT>::Decode(const std::vector<uint8_t>& compressedData)
	{
		ResetVariables();
		m_BinReader = BinIO::BinReader(std::make_shared<BinIO::MemoryReader>(compressedData));
		m_High = g_Max;
		m_Low = m_Scale = CountT(0);

		DecodeHeader();
		std::vector<SymT> outputBuffer(m_SymbolCounter.GetNumSymbols());
		InitializeDecodeBuffer();

		for (CountT i = 0; i < m_SymbolCounter.GetNumSymbols(); ++i) {
			outputBuffer[i] = DecodeStepImpl();
		}
		
		return outputBuffer;
	}

	template<typename SymT, CountSet CountT>
	inline std::vector<uint8_t> ACe123<SymT, CountT>::AdaptiveEncode(const std::vector<SymT>& text, const std::vector<SymT>& alphabet)
	{
		throw std::logic_error("Not implemented");
		return std::vector<uint8_t>();
	}

	template<typename SymT, CountSet CountT>
	inline std::vector<SymT> ACe123<SymT, CountT>::AdaptiveDecode(const std::vector<uint8_t>& compressedData)
	{
		throw std::logic_error("Not implemented");
		return std::vector<SymT>();
	}

	template<typename SymT, CountSet CountT>
	inline impl::ACe123SymbolCounting<SymT, CountT> ACe123<SymT, CountT>::DecodeHeader(const std::vector<uint8_t>& compressedData)
	{
		ResetVariables();
		m_BinReader = BinIO::BinReader(std::make_shared<BinIO::MemoryReader>(compressedData));
		DecodeHeader();
		return m_SymbolCounter;
	}
}
