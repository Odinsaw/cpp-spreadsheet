#pragma once
#pragma once

#include "common.h"
#include "formula.h"

#include <set>

class Cell : public CellInterface {
public:
	Cell();
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;

	bool IsEmpty() const;

	void ClearCache();
	bool CheckCacheValid();

	void SetParentCells();
	void SetChildCells();

	bool IsDependentOn(const Cell* cell) const;
private:

	class Impl {
	public:
		virtual Value GetValue() const = 0;
		virtual  std::string GetText() const = 0;
		virtual ~Impl() = default;
	};

	class EmptyImpl : public Impl {
	public:
		Value GetValue() const override;
		std::string GetText() const override;
	};

	class TextImpl : public Impl {
	public:
		explicit TextImpl(std::string text);

		Value GetValue() const override;
		std::string GetText() const override;

	private:
		std::string text_;
	};

	class FormulaImpl : public Impl {
	public:
		explicit FormulaImpl(std::string text);

		Value GetValue() const override;
		std::string GetText() const override;

	private:
		std::unique_ptr<FormulaInterface> formula_;
	};

	std::unique_ptr<Impl> impl_;

	std::set<const Cell*> parent_cells_;
	std::set<const Cell*> child_cells_;

	mutable std::optional<Value> cache_;

	bool CheckCircularDependancy() const;

	void ClearChildrenCache() const;
};
