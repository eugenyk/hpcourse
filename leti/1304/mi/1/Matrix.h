#pragma once
#include <vector>
#include <iomanip>
class Matrix {
public:
	using FormCard = std::pair<size_t, size_t>;
	Matrix(const FormCard& _formCard);
	const std::pair<size_t, size_t>& getFormCard() const;
	const size_t getLength() const;
	const std::vector<uint8_t>& getMatrix() const;
	std::vector<uint8_t>& getMatrix();
	void operator >> (std::ostream& out) const; 
	template<class Generator>
	Generator& operator<<(Generator& g);

private:
	FormCard formCard;
	std::vector<std::uint8_t> matrix;
};
