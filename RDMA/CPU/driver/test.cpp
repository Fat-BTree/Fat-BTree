#include "rdma_driver.h"

int main() {
  rdma_client rclient;
  rclient.rdma_client_setup("10.0.0.2");
}
