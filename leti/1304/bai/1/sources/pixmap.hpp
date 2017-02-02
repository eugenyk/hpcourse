//
//  Created by Aleksei Borisov (alekseiborisov@ilexbor.com) on 29.10.2016.
//  Copyright © 2016 Aleksei Borisov. All rights reserved.
//

#ifndef __HPCOURSE_LAB1_PIXMAP_HPP__
#define __HPCOURSE_LAB1_PIXMAP_HPP__

#include <stdint.h>
#include <vector>

namespace lex {

    class Pixmap {
    public:

        Pixmap();

        Pixmap(lex::Pixmap&& pixmap);

        Pixmap(const lex::Pixmap& pixmap);

        ~Pixmap();

        lex::Pixmap& operator=(lex::Pixmap&& pixmap);

        lex::Pixmap& operator=(const lex::Pixmap& pixmap);

        const ::uint8_t& at(const ::uint32_t row, const ::uint32_t column) const;

        ::uint8_t& at(const ::uint32_t row, const ::uint32_t column);

        void create(const ::uint32_t width, const uint32_t height);

        void destroy();
        
        const bool validate() const;

        const ::uint32_t width() const;

        const ::uint32_t height() const;

        const ::uint8_t* const data() const;

        ::uint8_t* const data();

    private:

        ::uint32_t m_width {0};
        ::uint32_t m_height {0};
        std::vector<::uint8_t> m_data;

    };

} // namespace lex

std::ofstream& operator<<(std::ofstream& out, const lex::Pixmap& pixmap);
std::ofstream& operator<<(std::ofstream& out, lex::Pixmap* pixmap);

#endif /* __HPCOURSE_LAB1_PIXMAP_HPP__ */
