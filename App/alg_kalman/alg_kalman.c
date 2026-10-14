#include "alg_kalman.h"

KalmanChannel channels;

// 初始化卡尔曼滤波器通道
void _kalman_single_assignment(KalmanChannel *channel, float Q, float R, float initial_X, float initial_P)
{
    channel->kalman_Q = Q;
    channel->kalman_R = R;
    channel->kalman_X = initial_X;
    channel->kalman_P = initial_P;
}

void alg_kalman_init(void)
{
    _kalman_single_assignment(&channels, 0.001f, 0.2f, 0, 1);
}	

// 卡尔曼滤波器函数，用于估计温度值
float alg_kalman_calc(KalmanChannel *channel, float kalman_Z)
{
	float X_ = channel->kalman_X; // 上一次的状态估计值
    float P_ = channel->kalman_P + channel->kalman_Q; // 更新估计误差协方差
    float K = P_ / (P_ + channel->kalman_R); // 计算卡尔曼增益
    channel->kalman_X = X_ + K * (kalman_Z - X_); // 更新状态估计值
    channel->kalman_P = P_ - K * P_; // 更新估计误差协方差
	return channel->kalman_X;
}
