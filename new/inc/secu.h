#ifndef __SECU_H__
#define __SECU_H__

extern int DESDecodeTrack( char *return_code,char *key_index,char *unit,char *indata,int iLen,char *outdata);
extern int DESGETKY(char *return_code, char *key_data, char *key_index);
extern int DESMACOP(char *return_code, char *mac_value, char *key_index, char *key_code, int length, char *data);
extern int DESPINTR(char *return_code, char *out_data, char *key_index1, char *key_code1,
             char *key_index2, char *key_code2, char *in_data, char *pan1, char *pan2);
extern int DESCalcTermMac(char *mackey_idx, char *key, char *srcBuf, int iLen, char *cMac, int sjflag);
extern int DESTrackCoding(char *keyIndex, char *key, char *buf, int len, int * outlen);
extern int DESPinConver(char *keyidx, char *key, char *pinBuf, char *pinkey_idx, char *pinkey, char *pan, char *buf);
extern int DESGETTMK(char *return_code, char *sek_index, char *tek_index, char *sek_tmk_data, char *tek_tmk_data);
extern int DESGETTMK2(char *return_code, char *sek_index, char *tek_index, char *sek_tmk_data, char *tek_tmk_data);
extern int DESTMKGETPIKMAK(char *return_code, char *sek_index1, char *sek_index2, char *tmk, char *sek_pikmak_data, char *tmk_pikmak_data) ;
extern int DESTMKGETPIKMAK2(char *return_code, char *sek_index1, char *sek_index2, char *tmk, char *sek_pikmak_data, char *tmk_pikmak_data, char *CheckValue, char *TMkOp, char *PIKMAKOp) ;
extern int DESTRANSPIN(char *return_code, char *sek_index1, char *sek_index2, char *pik1, char *pik2, char *in_pin_data, char *out_pin_dat);
extern int DESTRANSPIN2(char *return_code, char *sek_index1, char *sek_index2, char *pik1, char *pik2, char *in_pin_data, char *in_pan_data, char *out_pin_dat, char *out_pan_data);
extern int DESCPOSMAC2(char *return_code, char *sek_index, char *mak, int length, char *data, char *mac_value);
extern int DESCALCMAC(char *return_code, char *sek_index, char *mak, int length, char *data, char *mac_value);
extern int DES3CALCMAC(char *return_code, char *sek_index, char *mak, int length, char *data, char *mac_value);
extern int DESCHECKMAC(char *return_code, char *sek_index, char *mak, char *mac_value, int length, char *data);
extern int DESTRANSTMK(char *return_code, char *sek_index, char *tek_index, char *transbz, char *in_tmk, char *out_tmk);
extern int DESTRANSPIKMAK(char *return_code, char *sek_index1, char *sek_index2, char *tmk, char *transbz, char *in_pikmak, char *out_pikmak);
extern int DecTrackPrg(char *return_code, char *sek_index, char *tdk, int length, char *data, int *out_length, char *out_data);
extern int EncTrackPrg(char *return_code, char *sek_index, char *tdk, int length, char *data, int *out_length, char *out_data);
extern int GET_TMK(char *return_code, char *sek_index, char *tek_index, unsigned char flag, char *sek_tmk_data, char *tek_tmk_data, char *chk_tmk_data);
extern int GET_PIKorMAK(char *return_code, char *sek_index1, char *sek_index2, char *tmk, unsigned char tmk_flag, unsigned char pm_flag, char *sek_pikmak_data, char *tmk_pikmak_data, char *CheckValue) ;
extern int CALCMAC(char *return_code, char *sek_index, char *mak, unsigned char flag, int length, char *data, char *mac_value);
extern int TRANS_PIN(char *return_code, char *sek_index1, char *sek_index2, char *pik1, char *pik2, unsigned char flag1, unsigned char flag2, char *in_pin_data, char *in_pan_data, char *out_pin_dat, char *out_pan_data);
extern int Track_Decryption(char *return_code, char *sek_index, char *tdk, unsigned char flag, int length, char *data, int *out_length, char *out_data);
extern int _DESCALCMAC(char *return_code, char *sek_index, char *mak, int length, char *data, char *mac_value);



#endif