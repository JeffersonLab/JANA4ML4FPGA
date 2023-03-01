#pragma once

#include <stdint.h>

class CDaqRawData {

public:
    explicit CDaqRawData(const std::size_t create_data_len)
    {
        m_data.resize(create_data_len);
    }

    [[nodiscard]] const std::vector<int32_t>& RawData() const { return m_data; }

private:
    std::vector<int32_t> m_data;
};
