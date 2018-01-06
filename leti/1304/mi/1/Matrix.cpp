#include "stdafx.h"
#include "Matrix.h"

Matrix::Matrix(const FormCard& _formCard) : formCard(_formCard), matrix(_formCard.first * _formCard.second, 0){
}

std::vector<uint8_t>&Matrix:: getMatrix() {
	return matrix;
}
void Matrix::operator >> (std::ostream& out) const {
	for (size_t i = 0; i < formCard.first; ++i) {
		for (size_t j = 0; j < formCard.second; ++j) {
			out << std::setw(3) << (int)matrix[i * formCard.second + j] << " ";
		}
		out << "\n";
	}
}

const std::pair<size_t, size_t>& Matrix::getFormCard() const {
	return formCard;
}
const size_t Matrix::getLength() const {
	return formCard.first * formCard.second;
}
const std::vector<uint8_t>& Matrix::getMatrix() const {
	return matrix;
}
template<class Generator>
Generator& Matrix::operator<<(Generator& g) {
	std::transform(matrix.begin(),
		matrix.end(),
		matrix.begin(),
		[&g](std::uint8_t val) { return g(); });
	return g;
}
