// Copyright 2021 <github.com/razaqq>
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>


namespace PotatoAlert::Core {

enum class ColumnFormat
{
	Auto,
	Scientific,
	Fixed,
	Percent
};

enum class SortOrder
{
	Ascending,
	Descending
};

template<typename... RowTypes>
class AsciiTable
{
public:
	using Row = std::tuple<RowTypes...>;
	static constexpr size_t ColumnCount = sizeof...(RowTypes);

	explicit AsciiTable(const std::array<std::string, sizeof...(RowTypes)>&& headers, size_t cellPadding = 1, size_t staticColumnWidth = 0)
		: m_headers(headers), m_cellPadding(cellPadding), m_staticColumnWidth(staticColumnWidth)
	{}

	void AddRow(const RowTypes&... row)
	{
		m_rows.emplace_back(std::make_tuple(row...));
	}

	template<typename Stream>
	void Print(Stream& stream) const
	{
		const std::array<size_t, ColumnCount> columnWidths = ColumnWidths();

		auto printVLine = [&]()
		{
			stream << '+';
			for (size_t width : columnWidths)
			{
				stream << std::string(width + 2, '-');
				stream << '+';
			}
			stream << std::endl;
		};

		printVLine();

		// print the header
		stream << '|';
		for (size_t i = 0; i < ColumnCount; i++)
		{
			size_t half = columnWidths[i] / 2;
			half -= m_headers[i].size() / 2;

			stream << std::string(m_cellPadding, ' ') << std::setw(columnWidths[i]) << std::left
			       << std::string(half, ' ') + m_headers[i] << std::string(m_cellPadding, ' ') << '|';
		}
		stream << std::endl;

		printVLine();

		// print the rows
		for (const auto& row : m_rows)
		{
			stream << '|';
			size_t i = 0;
			TupleForEach(row, [&]<typename ValueType>(const ValueType& value)
			{
				if (m_hasPrecision)
				{
					stream << std::setprecision(m_precision[i]);
				}

				if (m_hasColumnFormat)
				{
					switch (m_columnFormat[i])
					{
					 case ColumnFormat::Auto:
						break;
					 case ColumnFormat::Scientific:
						stream << std::scientific;
						break;
					 case ColumnFormat::Fixed:
						stream << std::fixed;
						break;
					 case ColumnFormat::Percent:
						stream << std::fixed << std::setprecision(2);
						break;
					 default:
						break;
					}
				}

				constexpr decltype(&std::right) align = std::is_arithmetic_v<std::decay_t<ValueType>> ? std::right : std::left;

				stream << std::string(m_cellPadding, ' ') << std::setw(columnWidths[i])
				       << align << value << std::string(m_cellPadding, ' ') << '|';

				if (m_hasColumnFormat)
				{
					stream << std::defaultfloat;
				}

				i++;
			});
			stream << std::endl;
		}

		printVLine();
	}

	template<size_t Column>
	void SortByColumn(SortOrder sort = SortOrder::Descending)
	{
		static_assert(Column < ColumnCount, "Column index out of range");
		static_assert(
				requires(std::tuple_element_t<Column, Row> const& x) { x <=> x; }, "Value does not support sorting");

		if (sort == SortOrder::Ascending)
		{
			std::ranges::sort(m_rows, [&](const Row& a, const Row& b)
			{
				return std::get<Column>(a) < std::get<Column>(b);
			});
		}
		else
		{
			std::ranges::sort(m_rows, [&](const Row& a, const Row& b)
			{
				return std::get<Column>(a) > std::get<Column>(b);
			});
		}
	}

	void SetCellPadding(size_t cellPadding)
	{
		m_cellPadding = cellPadding;
	}

	void SetColumnFormat(const std::array<ColumnFormat, sizeof...(RowTypes)>& format)
	{
		m_columnFormat = format;
	}

	void SetColumnPrecision(const std::array<size_t, sizeof...(RowTypes)>& precision)
	{
		m_precision = precision;
	}

private:
	[[nodiscard]] std::array<size_t, sizeof...(RowTypes)> ColumnWidths() const
	{
		std::array<size_t, ColumnCount> widths{};

		for (size_t i = 0; i < ColumnCount; i++)
		{
			widths[i] = m_headers[i].size();
		}

		for (const Row& row : m_rows)
		{
			auto columnWidths = ColumnWidthsRow(row);
			for (size_t i = 0; i < ColumnCount; i++)
			{
				if (columnWidths[i] > widths[i])
				{
					widths[i] = columnWidths[i];
				}
			}
		}
		return widths;
	}

	template<typename Tuple, typename Func>
	static constexpr void TupleForEach(Tuple&& t, Func&& f)
	{
		std::apply([&f](auto&&... args)
		{
			(f(std::forward<decltype(args)>(args)), ...);
		}, std::forward<decltype(t)>(t));
	}

	template<typename T>
	[[nodiscard]] constexpr size_t DataWidth(T&& value) const
	{
		constexpr bool hasSize = requires(T && t) { t.size(); };

		if constexpr (std::is_integral_v<std::decay_t<decltype(value)>>)
		{
			return value == 0 ? 1 : std::log10(value) + 1;
		}
		else if constexpr (hasSize)
		{
			return value.size();
		}
	}

	[[nodiscard]] std::array<size_t, sizeof...(RowTypes)> ColumnWidthsRow(const Row& row) const
	{
		std::array<size_t, ColumnCount> sizes{};
		size_t i = 0;

		TupleForEach(row, [&](const auto& value)
		{
			if (m_hasColumnFormat && m_columnFormat[i] == ColumnFormat::Percent)
			{
				sizes[i] = 6;
			}
			else
			{
				sizes[i] = DataWidth(value);
			}
			i++;
		});

		return sizes;
	}

	std::array<std::string, ColumnCount> m_headers;
	std::vector<Row> m_rows;
	size_t m_cellPadding;
	size_t m_staticColumnWidth;
	std::array<ColumnFormat, ColumnCount> m_columnFormat;
	bool m_hasColumnFormat = false;
	std::array<size_t, ColumnCount> m_precision;
	bool m_hasPrecision = false;
};

}  // namespace PotatoAlert::Core
