#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

bool PositionIsCorrect(Position pos) {
	if (pos.col < 0 || pos.row < 0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS) {
		return false;
	}
	return true;
}

bool IsInsidePrintZone(Position pos, Size size) {
	return pos.row < size.rows&& pos.col < size.cols;
}

void Sheet::CheckPosition(Position pos) const {
	if (!PositionIsCorrect(pos)) {
		throw InvalidPositionException("Invalid position!"s);
	}
}

void Sheet::CheckCircularDependency(Position pos, const CellInterface* cell) const {
	if (cell == nullptr) {
		return;
	}

	for (const Position& parent_pos : cell->GetReferencedCells()) {
		if (pos == parent_pos) {
			throw CircularDependencyException("Circular dependency found!");
		}
		CheckCircularDependency(pos, GetCell(parent_pos));
	}
}

void Sheet::SetChildCells(Position pos, std::unique_ptr<Cell>& cell) {
	for (const Position& parent_pos : cell->GetReferencedCells()) {
		if (!GetCell(parent_pos)) {
			SetCell(parent_pos, {});
		}
		dynamic_cast<Cell*>(GetCell(parent_pos))->SetChildCell(pos);
	}
}

void Sheet::SetCell(Position pos, std::string text) {
	CheckPosition(pos);

	auto temp_cell = std::make_unique<Cell>(*this);
	temp_cell->Set(text);
	CheckCircularDependency(pos, temp_cell.get());

	if (pos.row >= static_cast<int>(sheet_.size())) {
		sheet_.resize(pos.row + 1);
	}
	if (pos.col >= static_cast<int>(sheet_.at(pos.row).size())) {
		sheet_.at(pos.row).resize(pos.col + 1);
	}
	size_.cols = std::max(size_.cols, pos.col + 1);
	size_.rows = std::max(size_.rows, pos.row + 1);

	auto& cell = sheet_.at(pos.row).at(pos.col);
	if (cell) {
		cell->Clear();
	}
	else {
		++non_empty_cols[pos.col];
		++non_empty_rows[pos.row];
	}
	cell = std::move(temp_cell);

	SetChildCells(pos, cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
	CheckPosition(pos);

	if (pos.row < static_cast<int>(sheet_.size()) && pos.col < static_cast<int>(sheet_.at(pos.row).size())
		&& sheet_.at(pos.row).at(pos.col)) {
		return sheet_.at(pos.row).at(pos.col).get();

	}
	else if (IsInsidePrintZone(pos, size_)) {
		const_cast<Sheet*>(this)->SetCell(pos, {});
		return sheet_.at(pos.row).at(pos.col).get();
	}
	else {
		return nullptr;
	}
}
CellInterface* Sheet::GetCell(Position pos) {
	return const_cast<CellInterface*>(std::as_const(*this).GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
	CheckPosition(pos);

	if (pos.row < static_cast<int>(sheet_.size()) && pos.col < static_cast<int>(sheet_.at(pos.row).size())) {
		sheet_.at(pos.row).at(pos.col)->Clear();
		--non_empty_cols[pos.col];
		--non_empty_rows[pos.row];

		if (non_empty_cols[pos.col] == 0) {
			non_empty_cols.erase(pos.col);
		}

		if (non_empty_rows[pos.row] == 0) {
			non_empty_rows.erase(pos.row);
		}

		if (non_empty_cols.empty() || non_empty_rows.empty()) {
			sheet_.clear();
			size_ = { 0, 0 };
			return;
		}

		if (pos.row == size_.rows - 1 && !non_empty_rows.count(pos.row)) {
			int new_height = 1 + non_empty_rows.rbegin()->first;
			sheet_.resize(new_height);
			size_.rows = new_height;
		}

		if (pos.col == size_.cols - 1 && !non_empty_cols.count(pos.col)) {
			int new_width = 1 + non_empty_cols.rbegin()->first;

			for (auto& row : sheet_) {
				if (static_cast<int>(row.size()) > new_width) {
					row.resize(new_width);
				}
			}
			size_.cols = new_width;
		}
	}
}

Size Sheet::GetPrintableSize() const {
	return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
	Print(output, &Cell::GetValue);
}
void Sheet::PrintTexts(std::ostream& output) const {
	Print(output, &Cell::GetText);
}

std::unique_ptr<SheetInterface> CreateSheet() {
	return std::make_unique<Sheet>();
}

void PrintEmpty(std::ostream& output, int num) {
	for (int i = 0; i < num - 1; ++i) {
		output << '\t';
	}
}

void Sheet::ClearCellCache(Position pos) {
	CheckPosition(pos);

	if (pos.row < static_cast<int>(sheet_.size()) && pos.col < static_cast<int>(sheet_.at(pos.row).size())
		&& sheet_.at(pos.row).at(pos.col)) {
		sheet_.at(pos.row).at(pos.col)->ClearCache();
	}
}