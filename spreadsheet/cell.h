#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    bool IsEmpty() const;

    void ClearCache();
    bool CheckCacheValid();

    void SetParentCell(const Position cell);
    void SetChildCell(const Position cell);

    bool IsDependentOn(const Position cell) const;
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

	class Impl {
	public:
		virtual Value GetValue() const = 0;
		virtual  std::string GetText() const = 0;
		virtual std::vector<Position> GetReferencedCells() const = 0;
		virtual ~Impl() = default;
	};

	class EmptyImpl : public Impl {
	public:
		Value GetValue() const override;
		std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;
	};

	class TextImpl : public Impl {
	public:
		explicit TextImpl(std::string text);

		Value GetValue() const override;
		std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;

	private:
		std::string text_;
	};

	class FormulaImpl : public Impl {
	public:
		explicit FormulaImpl(std::string text, Sheet& sheet, std::optional<Value>& cache);

		Value GetValue() const override;
		std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;

	private:
		std::unique_ptr<FormulaInterface> formula_;
		Sheet& sheet_;
		std::optional<Value>& cache_;
	};

	std::unique_ptr<Impl> impl_;
	Sheet& sheet_;

	std::set<Position> parent_cells_;
	std::set<Position> child_cells_;

	mutable std::optional<Value> cache_;

	void ClearChildrenCache() const;
};