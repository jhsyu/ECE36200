// The global variables used for this program.
// autotest may change any of these data before invoking
// your subroutines on them.
int sum = 0;
int source[]={3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322};
char str[] = "hello, 01234 world! 56789=-";

void intsub()
{
    for(int i = 0; i < 10; i++) {
        if (sum < 25)
            sum += source[i] + source[i+1];
        else
            sum -= 2 * source[i];
    }
}

void charsub()
{
    for(int x=0; str[x] != '\0' && str[x+1] != '\0'; x = x + 2) {
        if (str[x] >= 'a' && str[x] <= 'z') { // 'a' == 0x61, 'z' == 0x7a
            char tmp = str[x];
            str[x] = str[x+1];
            str[x+1] = tmp;
        }
    }
}

const char login[] = "xyz";
void main()
{
  //autotest();
  intsub();
  charsub();
}
