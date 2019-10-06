NFSClient *client_ptr;

static int nfs_mknod(const char *pathname, mode_t mode, dev_t dev)
{
  client_ptr->NFSPROC_MKNOD(pathname, mode, dev);
  return 0;
}

static void *nfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
}


static struct fuse_operations nfs_oper = {
        .mknod          = nfs_mknod
};

int main(int argc, char **argv)
{
  NFSClient *client_ptr = new NFSClient(
    grpc::CreateChannel("127.0.0.1:50055", grpc::InsecureChannelCredentials())
  );

  return 0;
}
