//
//  Created by Aleksei Borisov (alekseiborisov@ilexbor.com) on 29.10.2016.
//  Copyright © 2016 Aleksei Borisov. All rights reserved.
//

#include "pixmap.hpp"
#include <random>
#include <fstream>

lex::Pixmap::Pixmap() {}

lex::Pixmap::Pixmap(lex::Pixmap&& pixmap) {
    m_width = pixmap.m_width;
    m_height = pixmap.m_height;
    m_data = std::move(pixmap.m_data);
    pixmap.m_width = 0;
    pixmap.m_height = 0;
}

lex::Pixmap::Pixmap(const lex::Pixmap& pixmap) {
    lex::Pixmap::create(pixmap.m_width, pixmap.m_height);
}

lex::Pixmap::~Pixmap() {
    lex::Pixmap::destroy();
}

lex::Pixmap& lex::Pixmap::operator=(lex::Pixmap&& pixmap) {
    if(&pixmap != this) {
        lex::Pixmap::destroy();
        ::new(this) lex::Pixmap {std::move(pixmap)};
    }
    return (*this);
}

lex::Pixmap& lex::Pixmap::operator=(const lex::Pixmap& pixmap) {
    if(&pixmap != this) {
        ::new(this) lex::Pixmap {pixmap};
    }
    return (*this);
}

const ::uint8_t& lex::Pixmap::at(const ::uint32_t row, const ::uint32_t column) const {
    return m_data.at(row*m_width + column);
}

::uint8_t& lex::Pixmap::at(const ::uint32_t row, const ::uint32_t column) {
    return m_data.at(row*m_width + column);
}

void lex::Pixmap::create(const ::uint32_t width, const ::uint32_t height) {
    if(width > 0 && height > 0) {

        lex::Pixmap::destroy();
        
        const ::size_t size = width * height;
        m_data.reserve(size);

        m_width = width;
        m_height = height;

        std::random_device random_device;
        std::mt19937 generator(random_device());
        std::uniform_int_distribution<> distribution(0, 255);

        for(::size_t i = 0; i < size; i++) {
            m_data.push_back(distribution(generator));
        }
        
    }
}

void lex::Pixmap::destroy() {
    if(m_data.size() > 0 || m_width > 0 || m_height > 0) {
        m_data.clear();
        m_width = 0;
        m_height = 0;
    }
}

const bool lex::Pixmap::validate() const {
    return (m_data.size() > 0 && m_width > 0 && m_height > 0);
}

const ::uint32_t lex::Pixmap::width() const {
    return m_width;
}

const ::uint32_t lex::Pixmap::height() const {
    return m_height;
}

const ::uint8_t* const lex::Pixmap::data() const {
    return m_data.begin().base();
}

std::ofstream& operator<<(std::ofstream& out, const lex::Pixmap& pixmap) {
    out << "Image:\n";
    for(::uint32_t row = 0; row < pixmap.height(); row++) {
        for(::uint32_t column = 0; column < pixmap.width(); column++) {
            out << pixmap.at(row, column) << " ";
        }
        out << "\n";
    }
    return out;
}

std::ofstream& operator<<(std::ofstream& out, lex::Pixmap* pixmap) {
    if(pixmap != nullptr) {
        out << *pixmap;
    }
    return out;
}
