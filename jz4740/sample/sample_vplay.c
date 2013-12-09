#include "voice.h"
extern int IS_WRITE_PCM;
void replay_test(void)
{
  IS_WRITE_PCM=1;
  pcm_init();
  //record_play("/mnt/mtdblock/E_0.wav",IS_WRITE_PCM);
//  record_play("/mnt/mtdblock/E_1.wav",IS_WRITE_PCM);
//  wavplay("/mnt/mtdblock/E_1.wav");
	SetAudioVol(90);
  printf(">>>>>>> play /mnt/mtdblock/S_9_8.wav\n");
  wavplay("/mnt/mtdblock/S_9_8.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_12.wav\n");
  wavplay("/mnt/mtdblock/S_9_12.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_16.wav\n");
  wavplay("/mnt/mtdblock/S_9_16.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_22.wav\n");
  wavplay("/mnt/mtdblock/S_9_22.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_24.wav\n");
  wavplay("/mnt/mtdblock/S_9_24.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_32.wav\n");
  wavplay("/mnt/mtdblock/S_9_32.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_44.wav\n");
  wavplay("/mnt/mtdblock/S_9_44.wav");
  printf(">>>>>>> play /mnt/mtdblock/S_9_48.wav\n");
  wavplay("/mnt/mtdblock/S_9_48.wav");
// record_play("/mnt/mtdblock/E_1.wav",IS_WRITE_PCM);
//  record_play("startup.wav",IS_WRITE_PCM);
//  record_play("ring.wav",IS_WRITE_PCM);
//  record_play(fname,IS_WRITE_PCM);
}

void record_replay_test ()
{
  IS_WRITE_PCM=0;
  pcm_init();
 // record_play("test.wav",IS_WRITE_PCM);

}

void playwav(void)
{
	replay_test();
}
