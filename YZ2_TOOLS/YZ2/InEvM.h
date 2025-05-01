#pragma once

class InEvM
{
public:
    typedef unsigned char value_type; // unsigned char

private:
    value_type*  m_read_file;
    unsigned int m_length;
    unsigned int m_Position;

public:
    InEvM(value_type* read_file, unsigned int length)
    {
        m_read_file = read_file;
        m_length = length;
        m_Position = 0;
    }

    //---------------------------
    //  Bit Data Input Event
public:
    int                                // out: Returns the input data size
        InEv_ReadData(                 // ---: ----------------------
            value_type* out_data_area, // out: Data start address
            const int& in_data_size    // in : Data Request Size
        )
    {
        int count = 0;
        for (size_t i = 0; i < in_data_size; i++)
        {
            if (m_Position >= m_length)
            {
                break;
            }

            out_data_area[i] = m_read_file[m_Position];
            m_Position++;
            count++;
        }
        return count;
    }
};

