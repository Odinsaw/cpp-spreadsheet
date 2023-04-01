#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
	:impl_(std::make_unique<EmptyImpl>()), sheet_(sheet)
{
}

Cell::~Cell() {}

void Cell::Set(std::string text = ""s) {
	if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
	}
	else if (text.front() == FORMULA_SIGN && text.length() > 1) {
		impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet_, cache_);
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
	}
	std::vector<Position> ref_cells = impl_->GetReferencedCells();
	parent_cells_ = std::set<Position>(std::make_move_iterator(ref_cells.begin()), std::make_move_iterator(ref_cells.end()));
}

void Cell::Clear() {
	ClearCache();
	child_cells_.clear();
	Set();
}

bool Cell::IsEmpty() const {
	return !impl_ || impl_->GetText() == std::string{};
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

std::optional<double> StringToDouble(const std::string& str) {
	std::stringstream str_d(str);
	double res;
	if (str_d >> res && str_d.eof()) {
		return res;
	}
	else {
		return std::nullopt;
	}
}

Cell::Value Cell::TextImpl::GetValue() const {
	if (!text_.empty() && text_.front() == ESCAPE_SIGN) {
		return text_.substr(1);
	}
	else if (std::optional<double> result = StringToDouble(text_)) {
		return result.value();
	}
	else {
		return GetText();
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
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

