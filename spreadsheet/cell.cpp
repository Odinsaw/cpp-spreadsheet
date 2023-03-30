#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
	:impl_(std::make_unique<EmptyImpl>()), sheet_(sheet)
{
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	if (!text.empty() && text.front() == '=' && text.length() > 1) {
		impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet_, cache_);
		std::vector<Position> ref_cells = impl_->GetReferencedCells();
		parent_cells_ = std::set<Position>(std::make_move_iterator(ref_cells.begin()), std::make_move_iterator(ref_cells.end()));
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
	}
}

void Cell::Clear() {
	ClearCache();
	child_cells_.clear();
	parent_cells_.clear();
	impl_ = std::make_unique<EmptyImpl>();
}

bool Cell::IsEmpty() const {
	if (!impl_ || impl_->GetText() == std::string{}) {
		return true;
	}
	return false;
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}
std::string Cell::GetText() const {
	if (!IsEmpty()) {
		return impl_->GetText();
	}
	else {
		return {};
	}
}

Cell::TextImpl::TextImpl(std::string text)
	:text_(text)
{
}

Cell::FormulaImpl::FormulaImpl(std::string text, Sheet& sheet, std::optional<Value>& cache)
	:formula_(ParseFormula(text)), sheet_(sheet), cache_(cache)
{
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

bool StringIsDouble(const char* str) {
	char* endptr = 0;
	strtod(str, &endptr);

	if (*endptr != '\0' || endptr == str)
		return false;
	return true;
}

Cell::Value Cell::TextImpl::GetValue() const {
	if (!text_.empty() && text_.front() == '\'') {
		return text_.substr(1);
	}
	else if (StringIsDouble(text_.data())) {
		return stod(text_);
	}
	else {
		return GetText();
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return '=' + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
	if (cache_) {
		CellInterface::Value val = *cache_;
		if (std::holds_alternative<double>(val)) {
			return std::get<double>(val);
		}
		else {
			return std::get<FormulaError>(val);
		}
	}

	auto res = formula_->Evaluate(sheet_);
	if (std::holds_alternative<double>(res)) {
		cache_ = std::get<double>(res);
		return std::get<double>(res);
	}
	else {
		cache_ = std::get<FormulaError>(res);
		return std::get<FormulaError>(res);
	}
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return {};
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return {};
}

std::string Cell::EmptyImpl::GetText() const {
	return {};
}

Cell::Value Cell::EmptyImpl::GetValue() const {
	return {};
}

std::vector<Position> Cell::GetReferencedCells() const {
	return std::vector<Position>(parent_cells_.begin(), parent_cells_.end());
}

bool Cell::IsReferenced() const {
	return !child_cells_.empty();
}

void Cell::ClearChildrenCache() const {
	for (const Position child_cell : child_cells_) {
		sheet_.ClearCellCache(child_cell);
	}
}

void Cell::ClearCache() {
	cache_.reset();
	ClearChildrenCache();
}

bool Cell::CheckCacheValid() {
	return cache_.has_value();
}

void Cell::SetParentCell(const Position cell) {
	parent_cells_.insert(cell);
}

void Cell::SetChildCell(const Position cell) {
	child_cells_.insert(cell);
}

bool Cell::IsDependentOn(const Position cell) const {
	return parent_cells_.count(cell);
}

