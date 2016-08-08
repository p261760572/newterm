
#ifndef DB_H_
#define DB_H_


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define DB_ERROR() {                                                                     \
    unsigned char err_msg[512];                                                          \
    size_t msg_buf_len = 0, msg_len = 0;                                                 \
                                                                                         \
    msg_buf_len = sizeof(err_msg);                                                       \
    memset(err_msg, 0, sizeof(err_msg));                                                 \
    sqlglm(err_msg, &msg_buf_len, &msg_len);                                             \
    dcs_log(0, 0, "at %s(%s:%d) %.*s",__FUNCTION__,__FILE__,__LINE__, msg_len, err_msg); \
}

extern char *strcpy_s(char *dest, const char *src, size_t n);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* DB_H_ */
