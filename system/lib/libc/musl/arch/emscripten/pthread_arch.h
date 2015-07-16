static inline struct pthread *__pthread_self(void) { return pthread_self(); }

#define TP_ADJ(p) (p)

#define CANCEL_REG_IP 16
