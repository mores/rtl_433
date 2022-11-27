/**

rtl_433 -f 319.56M -X "n=ge,m=OOK_PWM,s=330,l=1000,r=1500"

FCC ID: QOB-PT458

House Code D
   Channel 0
      On:  ea af a8 80  : 11101010 10101111 10101000 1
      Off: ea af ab 80  : 11101010 10101111 10101011 1
                           *                      **

   Channel 1
      On:  aa af a8 80  : 10101010 10101111 10101000 1
      Off: aa af ab 80  : 10101010 10101111 10101011 1
                           *                      **
*/

#include "decoder.h"

static void print_bitrow(char *raw_data, uint8_t const *bitrow, unsigned bit_len, int always_binary)
{
    sprintf(raw_data, "{%2u} ", bit_len);
    for (unsigned col = 0; col < (bit_len + 7) / 8; ++col) {
        sprintf(raw_data + strlen(raw_data), "%02x ", bitrow[col]);
    }
    sprintf(raw_data + strlen(raw_data), " : " );
    // Print binary values also?
    if (always_binary || bit_len <= BITBUF_MAX_PRINT_BITS) {
        for (unsigned bit = 0; bit < bit_len; ++bit) {
            if (bitrow[bit / 8] & (0x80 >> (bit % 8))) {
                sprintf(raw_data + strlen(raw_data), "1");
            }
            else {
                sprintf(raw_data + strlen(raw_data), "0");
            }
            if ((bit % 8) == 7) // Add byte separators
                sprintf(raw_data + strlen(raw_data), " ");
        }
    }
}

static int ge_smartremote_plus_callback(r_device *decoder, bitbuffer_t *bitbuffer)
{
    data_t *data;
    uint8_t *bitrow;
    char raw_data[BITBUF_ROWS * BITBUF_COLS * 2 + 1];
    unsigned row = 0;
    int channel = -1;

    //bitbuffer_debug(bitbuffer);

    if (bitbuffer->bits_per_row[row] < 25)
        return DECODE_ABORT_EARLY;

    print_bitrow(raw_data, bitbuffer->bb[row], bitbuffer->bits_per_row[row], 1);

    bitrow    = bitbuffer->bb[row];

    if (bitrow[0] == 0xea)
        channel = 0;

    if (bitrow[0] == 0xaa)
        channel = 1;

    data = data_make(
            "model",        "",             DATA_STRING, "GE Smartremote-RF108",
            "channel",      "",             DATA_INT, channel,
            "state",        "",             DATA_STRING, (bitrow[2] == 0xa8) ? "on" : "off",
            "data",         "Raw Data",     DATA_STRING, raw_data,
            NULL);
    /* clang-format on */
    decoder_output_data(decoder, data);

    return 1;
}

static char *output_fields[] = {
        "model",
        "channel",
        "state",
        "data",
        NULL,
};

r_device ge_smartremote_plus = {
        .name        = "GE Smartremote Plus RF108",
        .modulation  = OOK_PULSE_PWM,
        .short_width = 330,          // Threshold between short and long pulse [us]
        .long_width  = 1000,         // Maximum gap size before new row of bits [us]
        .reset_limit = 1500,         // Maximum gap size before End Of Message [us]
        .decode_fn   = &ge_smartremote_plus_callback,
        .fields      = output_fields,
};
