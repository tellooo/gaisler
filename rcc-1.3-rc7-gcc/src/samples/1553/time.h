#ifndef __TIME_H_
#define __TIME_H_

int init_spwcuc(int tx, char *grspw_devname);
void spwcuc_handling(void);

int init_ctm(int tx, int spw);
void ctm_handling(void);

#endif /* __TIME_H_ */
