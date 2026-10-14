#ifndef __HY_KALMAN_H__
#define __HY_KALMAN_H__

typedef struct {
    float kalman_Q; // 过程噪声协方差
    float kalman_R; // 测量噪声协方差
    float kalman_X; // 状态估计值
    float kalman_P; // 估计误差协方差
} KalmanChannel;

extern KalmanChannel channels;

void hy_kalman_init(void);
float hy_kalman_calc(KalmanChannel *channel, float kalman_Z);

#endif
