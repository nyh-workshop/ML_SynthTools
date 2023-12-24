/*
 * Copyright (c) 2023 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file i2s_interface.ino
 * @author Marcel Licence
 * @date 13.10.2021
 *
 * @brief  this file includes all required function to setup and drive the i2s interface
 *         You can use the internal DAC by defining I2S_NODAC in your configuration
 * @see https://youtu.be/zoajxQ7X0Gk
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef SAMPLE_RATE


#ifdef ML_SYNTH_INLINE_DECLARATION

#ifndef ESP32S3_I2S
void setup_i2s();
#else
void setup_i2s_ESP32S3();
#endif
bool i2s_write_stereo_samples_i16(const int16_t *fl_sample, const int16_t *fr_sample, const int buffLen);
bool i2s_write_stereo_samples_buff(const float *fl_sample, const float *fr_sample, const int buffLen);
void i2s_read_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen);
void i2s_read_stereo_samples_buff(int16_t *fl_sample, int16_t *fr_sample, const int buffLen);

#endif /* ML_SYNTH_INLINE_DECLARATION */


#ifdef ML_SYNTH_INLINE_DEFINITION


#ifdef ESP32

#ifndef ESP32S3_I2S
#include <driver/i2s.h>
#else
#include <driver/i2s_std.h>
#endif


#ifdef I2S_NODAC
#ifndef SOC_I2S_SUPPORTS_DAC
#error internal dac not supported by your current configuration
/* this message appears in case you cannot use the I2S interface to push audio data to the internal DAC */
#endif
#endif


#ifdef OUTPUT_SAW_TEST
int16_t sampleDataI16SawTest[SAMPLE_BUFFER_SIZE];
float sampleDataFSawTest[SAMPLE_BUFFER_SIZE];
#endif


#ifdef SAMPLE_SIZE_16BIT
#define BYTES_PER_SAMPLE    2
#define MULTIPLIER_CONST    (16383.0f)
#define SAMPLE_DATA_TYPE    int16_t
#endif
#ifdef SAMPLE_SIZE_32BIT
#define BYTES_PER_SAMPLE    4
#define MULTIPLIER_CONST    (1073741823.0f)
#define SAMPLE_DATA_TYPE    int32_t
#endif


#ifdef SAMPLE_SIZE_32BIT
union sampleTUNT
{
    uint64_t sample;
    int32_t ch[2];
};
#endif

#ifdef SAMPLE_SIZE_24BIT
union sampleTUNT
{
    int32_t ch[2];
    uint8_t bytes[8];
};
#endif

#ifdef SAMPLE_SIZE_16BIT
union sampleTUNT
{
    uint32_t sample;
    int16_t ch[2];
};
#endif

/*
 * no dac not tested within this code
 * - it has the purpose to generate a quasy analog signal without a DAC
 */
//#define I2S_NODAC


const i2s_port_t i2s_port_number = I2S_NUM_0;

#ifdef ESP32S3_I2S
static i2s_chan_handle_t tx_chan; // I2S tx channel handler
#endif

/*
 * please refer to https://www.hackster.io/janost/audio-hacking-on-the-esp8266-fa9464#toc-a-simple-909-drum-synth-0
 * for the following implementation
 */

bool i2s_write_sample_32ch2(uint64_t sample)
{
    static size_t bytes_written = 0;
	
#ifndef ESP32S3_I2S
    i2s_write((i2s_port_t)i2s_port_number, (const char *)&sample, 8, &bytes_written, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifdef SAMPLE_SIZE_24BIT

bool i2s_write_sample_24ch2(uint8_t *sample);

bool i2s_write_sample_24ch2(uint8_t *sample)
{
    static size_t bytes_written1 = 0;
    static size_t bytes_written2 = 0;
#ifndef ESP32S3_I2S
    i2s_write(i2s_port_number, (const char *)&sample[1], 3, &bytes_written1, portMAX_DELAY);
    i2s_write(i2s_port_number, (const char *)&sample[5], 3, &bytes_written2, portMAX_DELAY);
#else
    //#error ESP32S3 not supported at this time!
#endif

    if ((bytes_written1 + bytes_written2) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif

bool i2s_write_stereo_samples(const float *fl_sample, const float *fr_sample)
{
    static union sampleTUNT sampleDataU;

    /*
     * using RIGHT_LEFT format
     */
    sampleDataU.ch[0] = (SAMPLE_DATA_TYPE)(*fr_sample * MULTIPLIER_CONST);
    sampleDataU.ch[1] = (SAMPLE_DATA_TYPE)(*fl_sample * MULTIPLIER_CONST);

    size_t bytes_written = 0;
	
#ifndef ESP32S3_I2S
    i2s_write(i2s_port_number, (const char *)&sampleDataU.sample, 2 * BYTES_PER_SAMPLE, &bytes_written, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifdef SAMPLE_SIZE_16BIT
bool i2s_write_stereo_samples_i16(const int16_t *fl_sample, const int16_t *fr_sample)
{
    size_t bytes_written = 0;

    static union sampleTUNT sampleDataU;

    sampleDataU.ch[0] = *fl_sample;
    sampleDataU.ch[1] = *fr_sample;
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif

#ifndef ESP32S3_I2S
    i2s_write(i2s_port_number, (const char *)&sampleDataU.sample, 4, &bytes_written, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif
	
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif
    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

#ifdef SAMPLE_SIZE_16BIT
bool i2s_write_stereo_samples_i16(const int16_t *fl_sample, const int16_t *fr_sample, const int buffLen)
{
    size_t bytes_written = 0;

    static union sampleTUNT sampleDataU[SAMPLE_BUFFER_SIZE];

#ifdef OUTPUT_SAW_TEST
    for (int n = 0; n < buffLen; n++)
    {
        sampleDataU[n].ch[1] = sampleDataI16SawTest[n];
        sampleDataU[n].ch[0] = sampleDataI16SawTest[n];
    }
#else
    for (int n = 0; n < buffLen; n++)
    {
        sampleDataU[n].ch[1] = fr_sample[n];
        sampleDataU[n].ch[0] = fl_sample[n];
    }
#endif

#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif

#ifndef ESP32S3_I2S
    i2s_write(i2s_port_number, (const char *)&sampleDataU[0].sample, 2 * BYTES_PER_SAMPLE * buffLen, &bytes_written, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif
	
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

#ifdef SAMPLE_BUFFER_SIZE
bool i2s_write_stereo_samples_buff(const float *fl_sample, const float *fr_sample, const int buffLen)
{
    static union sampleTUNT sampleDataU[SAMPLE_BUFFER_SIZE];

    for (int n = 0; n < buffLen; n++)
    {
#ifdef ES8388_ENABLED
        /*
         * using LEFT_RIGHT format
         */
        sampleDataU[n].ch[1] = (SAMPLE_DATA_TYPE)(fr_sample[n] * MULTIPLIER_CONST);
        sampleDataU[n].ch[0] = (SAMPLE_DATA_TYPE)(fl_sample[n] * MULTIPLIER_CONST);
#else /* ES8388_ENABLED */
        /*
         * using RIGHT_LEFT format
         */
#ifdef SAMPLE_SIZE_16BIT
#ifdef I2S_NODAC
        float fr_sampl = fr_sample[n];
        float fl_sampl = fl_sample[n];

        /* shift signal */
        fr_sampl *= 0.5;
        fl_sampl *= 0.5;
        fr_sampl += 0.5f;
        fl_sampl += 0.5f;

        /* limit */
        fr_sampl = fr_sampl > 1.0f ? 1.0f : fr_sampl;
        fl_sampl = fl_sampl > 1.0f ? 1.0f : fl_sampl;
        fr_sampl = fr_sampl < 0.0f ? 0.0f : fr_sampl;
        fl_sampl = fl_sampl < 0.0f ? 0.0f : fl_sampl;

        /* convert */
        sampleDataU[n].ch[0] = uint16_t(fl_sampl * 0xFFFF); /* DAC_1 */
        sampleDataU[n].ch[1] = uint16_t(fr_sampl * 0xFFFF); /* DAC_2 */
#else
        sampleDataU[n].ch[0] = int16_t(fr_sample[n] * 16383.0f); /* some bits missing here */
        sampleDataU[n].ch[1] = int16_t(fl_sample[n] * 16383.0f);
#endif
#endif
#ifdef SAMPLE_SIZE_32BIT
        sampleDataU[n].ch[0] = (SAMPLE_DATA_TYPE)(fr_sample[n] * MULTIPLIER_CONST);
        sampleDataU[n].ch[1] = (SAMPLE_DATA_TYPE)(fl_sample[n] * MULTIPLIER_CONST);
#endif
#endif /* ES8388_ENABLED */
    }

#ifdef OUTPUT_SAW_TEST
    for (int n = 0; n < buffLen; n++)
    {
        sampleDataU[n].ch[1] = sampleDataI16SawTest[n];
        sampleDataU[n].ch[0] = sampleDataI16SawTest[n];
    }
#endif

    static size_t bytes_written = 0;

#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif
#ifndef ESP32S3_I2S
    i2s_write(i2s_port_number, (const char *)&sampleDataU[0].sample, 2 * BYTES_PER_SAMPLE * buffLen, &bytes_written, portMAX_DELAY);
#else
	ESP_ERROR_CHECK(i2s_channel_write(tx_chan, (const char *)&sampleDataU[0].sample, 2 * BYTES_PER_SAMPLE * buffLen, &bytes_written, portMAX_DELAY));
#endif
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif /* #ifdef SAMPLE_BUFFER_SIZE */

void i2s_read_stereo_samples(float *fl_sample, float *fr_sample)
{
    static size_t bytes_read = 0;

    static union sampleTUNT sampleData;
#ifndef ESP32S3_I2S
    i2s_read(i2s_port_number, (char *)&sampleData.sample, 4, &bytes_read, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif
    /*
     * using RIGHT_LEFT format
     */
    *fr_sample = ((float)sampleData.ch[0] * (5.5f / 65535.0f));
    *fl_sample = ((float)sampleData.ch[1] * (5.5f / 65535.0f));
}

#ifdef SAMPLE_BUFFER_SIZE
void i2s_read_stereo_samples_buff(int16_t *fl_sample, int16_t *fr_sample, const int buffLen)
{
#ifdef I2S_DIN_PIN
    static size_t bytes_read = 0;

    static union sampleTUNT sampleData[SAMPLE_BUFFER_SIZE];

#ifndef ESP32S3_I2S
    i2s_read(i2s_port_number, (char *)&sampleData[0].sample, 2 * BYTES_PER_SAMPLE * buffLen, &bytes_read, portMAX_DELAY);
#else
	//#error ESP32S3 not supported at this time!
#endif 

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
        fr_sample[n] = ((float)sampleData[n].ch[0]);
        fl_sample[n] = ((float)sampleData[n].ch[1]);
    }
#endif
}

void i2s_read_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen)
{
#ifdef I2S_DIN_PIN
    static size_t bytes_read = 0;

    static union sampleTUNT sampleData[SAMPLE_BUFFER_SIZE];

#ifndef ESP32S3_I2S
    i2s_read(i2s_port_number, (char *)&sampleData[0].sample, 2 * BYTES_PER_SAMPLE * buffLen, &bytes_read, 0);
#else
	//#error ESP32S3 not supported at this time!
#endif    

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */

        fr_sample[n] = ((float)sampleData[n].ch[0] * (1.0f / MULTIPLIER_CONST));
        fl_sample[n] = ((float)sampleData[n].ch[1] * (1.0f / MULTIPLIER_CONST));
    }
#endif
}
#endif /* #ifdef SAMPLE_BUFFER_SIZE */

/*
 * i2s configuration
 */
#ifndef ESP32S3_I2S

#ifdef I2S_NODAC
static const i2s_config_t i2s_configuration =
{
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = SAMPLE_RATE * 1,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // only the top 8 bits will actually be used by the internal DAC, but using 8 bits straight away seems buggy
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // always use stereo output. mono seems to be buggy, and the overhead is insignifcant on the ESP32
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB), // this appears to be the correct setting for internal DAC and PT8211, but not for other dacs
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8, // 8*128 bytes of buffer corresponds to 256 samples (2 channels, see above, 2 bytes per sample per channel)
    .dma_buf_len = 64,
#ifdef I2S_USE_APLL
    .use_apll = true,
#else
    .use_apll = false,
#endif
};
#else
i2s_config_t i2s_configuration =
{
#ifdef I2S_DIN_PIN
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // | I2S_MODE_DAC_BUILT_IN
#else
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
#endif
    .sample_rate = SAMPLE_RATE,
#ifdef I2S_NODAC
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
#ifdef ARDUINO_RUNNING_CORE /* tested with arduino esp32 core version 2.0.2 */
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
#endif
#else
#ifdef SAMPLE_SIZE_32BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, /* the DAC module will only take the 8bits from MSB */
#endif
#ifdef SAMPLE_SIZE_24BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT, /* the DAC module will only take the 8bits from MSB */
#endif
#ifdef SAMPLE_SIZE_16BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
#endif
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
#ifdef ARDUINO_RUNNING_CORE /* tested with arduino esp32 core version 2.0.2 */
#ifdef MAX_98357A_ENABLED
    .communication_format = I2S_COMM_FORMAT_STAND_PCM_LONG,
#else
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#endif
#else
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
#endif
#endif
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 2,
    .dma_buf_len = SAMPLE_BUFFER_SIZE,
#ifdef I2S_USE_APLL
    .use_apll = true,
#else
    .use_apll = false,
#endif

#ifdef ARDUINO_RUNNING_CORE
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
#ifdef SAMPLE_SIZE_16BIT
    .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
#endif
#ifdef SAMPLE_SIZE_24BIT
    .bits_per_chan = I2S_BITS_PER_CHAN_24BIT,
#endif
#ifdef SAMPLE_SIZE_32BIT
    .bits_per_chan = I2S_BITS_PER_CHAN_32BIT,
#endif
#endif


};
#endif

#endif


#ifdef I2S_NODAC
#ifdef ESP8266
i2s_pin_config_t pins =
{
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = I2S_PIN_NO_CHANGE,
    .data_out_num = I2S_NODAC_OUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
};
#endif
#else

#ifndef ESP32S3_I2S
i2s_pin_config_t pins =
{
#ifdef ARDUINO_RUNNING_CORE
    .mck_io_num = I2S_PIN_NO_CHANGE,
#endif
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num = I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN,
#ifdef I2S_DIN_PIN
    .data_in_num = I2S_DIN_PIN,
#else
    .data_in_num = I2S_PIN_NO_CHANGE,
#endif
};
#endif

#endif

#ifndef ESP32S3_I2S
void setup_i2s()
{
    i2s_driver_install(i2s_port_number, &i2s_configuration, 0, NULL);
#ifdef I2S_NODAC
    i2s_set_pin(i2s_port_number, NULL);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    i2s_zero_dma_buffer(i2s_port_number);
#else
    i2s_set_pin(I2S_NUM_0, &pins);
#endif
    i2s_set_sample_rates(i2s_port_number, SAMPLE_RATE);
    i2s_start(i2s_port_number);
#ifdef ES8388_ENABLED
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
#endif
#ifndef I2S_NODAC
    Serial.printf("I2S configured using following pins:\n");
    Serial.printf("    BCLK,BCK: %d\n", pins.bck_io_num);
    Serial.printf("    WCLK,LCK: %d\n", pins.ws_io_num);
    Serial.printf("    DOUT: %d\n", pins.data_out_num);
    Serial.printf("    DIN: %d\n", pins.data_in_num);
#else
    Serial.printf("I2S configured using internal DAC (DAC_1, DAC_2 as output)\n");
#endif

#ifdef OUTPUT_SAW_TEST
    /*
       * base frequency: SAMPLE_FREQ / SAMPLE_BUFFER_SIZE
       * for example: Fs : 44100Hz, Lsb = 48 -> Freq: 918.75 Hz
       */
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        float saw = ((float)i * 2.0f) / ((float)SAMPLE_BUFFER_SIZE);
        sampleDataFSawTest[i] = saw;
        saw -= 1;
        saw *= 0x7FFF;
        sampleDataI16SawTest[i] = saw;
    }
#endif
}
#endif

#ifdef ESP32S3_I2S
void setup_i2s_ESP32S3()
{
	/* Setp 1: Determine the I2S channel configuration and allocate two channels one by one
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role
     * The tx and rx channels here are registered on different I2S controller,
     * only ESP32-C3, ESP32-S3 and ESP32-H2 allow to register two separate tx & rx channels on a same controller */
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));

    /* Step 2: Setting the configurations of standard mode and initialize each channels one by one
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_std.h' which can only be used in STD mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = I2S_BCLK_PIN,
            .ws   = I2S_WCLK_PIN,
            .dout = I2S_DOUT_PIN,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

	Serial.printf("ESP32S3 I2S ESP-IDF 5.x driver init! :D\n");
	
	Serial.printf("I2S configured using following pins:\n");
    Serial.printf("    BCLK,BCK: %d\n", tx_std_cfg.gpio_cfg.bclk);
    Serial.printf("    WCLK,LCK: %d\n", tx_std_cfg.gpio_cfg.ws);
    Serial.printf("    DOUT: %d\n", tx_std_cfg.gpio_cfg.dout);
}
#endif

#endif /* ESP32 */


#endif /* SAMPLE_RATE */


#endif /* ML_SYNTH_INLINE_DEFINITION */

