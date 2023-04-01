#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <ostream>

using namespace std::literals;

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
	std::visit(
		[&](const auto& x) {
			output << x;
		},
		value);
	return output;
}

void PrintEmpty(std::ostream& output, int num);

class Sheet : public SheetInterface {
public:
	~Sheet();

	void SetCell(Position pos, std::string text) override;

	const CellInterface* GetCell(Position pos) const override;
	CellInterface* GetCell(Position pos) override;

	void ClearCell(Position pos) override;

	Size GetPrintableSize() const override;

	void PrintValues(std::ostream& output) const override;
	void PrintTexts(std::ostream& output) const override;

	void ClearCellCache(Position pos);

private:
	using Row = std::vector <std::unique_ptr<Cell>>;
	using Id = int;

	template<typename T>
	void Print(std::ostream& output, T(Cell::*ty)() const) const {
		for (auto& row : sheet_) {
			for (size_t i = 0; i < row.size(); ++i) {
				auto& cell = row[i];
				if (cell) {
					const CellInterface::Value val = (*cell.*ty)();
					output << val;
				}
				if (static_cast<int>(i) < size_.cols - 1) {
					output << '\t';
				}
			}
			if (static_cast<int>(static_cast<int>(row.size())) < size_.cols) {
				PrintEmpty(output, size_.cols - static_cast<int>(row.size()));
			}
			output << '\n';
		}
	}

	void CheckPosition(Position pos) const;
	void CheckCircularDependency(Position pos, const CellInterface* cell) const;
	void SetChildCells(Position pos, std::unique_ptr<Cell>& cell);
	void ResizeTable(Position pos);
	std::unique_ptr<Cell>& AddNewCellToSheet(Position pos, std::unique_ptr<Cell>&& cell);

	std::vector<Row> sheet_;
	Size size_;
	std::map<Id, int> non_empty_cols;
	std::map<Id, int> non_empty_rows;
};