#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>

void PrintEmpty(std::ostream& output, int num);
std::ostream& operator<<(std::ostream& out, const CellInterface::Value& val);

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

	// Можете дополнить ваш класс нужными полями и методами


private:
	using Row = std::vector <std::unique_ptr<Cell>>;
	using Id = int;

	template<typename T>
	void Print(std::ostream& output, T(Cell::* ty)() const) const {
		for (auto& row : sheet_) {
			for (auto it = row.begin(); it != row.end(); ++it) {

				auto& cell = *it;
				if (cell) {
					CellInterface::Value val = (*cell.*ty)();
					output << val;
				}
				if (it != row.begin() + row.size() - 1) {
					output << '\t';
				}
			}
			if (static_cast<int>(row.size()) < size_.cols) {
				output << '\t';
				PrintEmpty(output, size_.cols - static_cast<int>(row.size()));
			}
			output << '\n';
		}
	}

	std::vector<Row> sheet_;
	Size size_;
	std::map<Id, int> non_empty_cols;
	std::map<Id, int> non_empty_rows;
};