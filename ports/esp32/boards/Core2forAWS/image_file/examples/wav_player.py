from M5Library import I2S
from machine import Pin
from Edukit import pmu
import io

BCK_PIN = Pin(12)
WS_PIN = Pin(0)
SDOUT_PIN = Pin(2)

class Speaker:

    F16B = I2S.B16
    F24B = I2S.B24
    F32B = I2S.B32

    CHN_LR = I2S.RIGHT_LEFT
    CHN_L = I2S.ONLY_LEFT
    CHN_R = I2S.ONLY_RIGHT

    def __init__(self):
        pass
    
    def playWAV(self, wav_file, rate=44100, dataf=F16B, channel=CHN_LR):
        if len(wav_file) < 25:
            try:
                wav = open(wav_file, 'rb')
            except Exception as e:
                print('Audio file open caught exception: {} {}'.format(type(e).__name__, e))
                return
        else:
            wav = io.BytesIO(len(wav_file))
            wav.write(wav_file)

        audio_out = I2S(
            I2S.NUM0,
            bck=BCK_PIN, ws=WS_PIN, sdout=SDOUT_PIN,
            standard=I2S.PHILIPS,
            mode=I2S.MASTER_TX,
            dataformat=dataf,
            channelformat=channel,
            samplerate=rate,
            dmacount=2, dmalen=256)

        # advance to first byte of Data section in WAV file
        wav.seek(44)

        # allocate sample arrays
        # memoryview used to reduce heap allocation in while loop
        wav_samples = bytearray(256)
        wav_samples_mv = memoryview(wav_samples)

        # continuously read audio samples from the WAV file
        # and write them to an I2S DAC
        pmu.setSpkEnable(True)
        try:
            while True:
                # try:
                num_read = wav.readinto(wav_samples_mv)
                num_written = 0
                if num_read == 0:
                    # pos = wav.seek(44)
                    # exit
                    break
                else:
                    while num_written < num_read:
                        num_written += audio_out.write(wav_samples_mv[num_written:num_read], timeout=10)
        except (KeyboardInterrupt, Exception) as e:
            print('Speaker caught exception: {} {}'.format(type(e).__name__, e))
            raise
        finally:
            wav.close()
            audio_out.deinit()
            pmu.setSpkEnable(False)

a = Speaker()
a.playWAV("res/example.wav")