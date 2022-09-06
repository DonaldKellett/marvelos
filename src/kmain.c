void kmain(void) {
  volatile int a = 4;
  volatile int b = 12;
  while (1) {
    volatile int c = a + b;
  }
}
