class Teste{
  public static int a, b;

  public static int c(){
    ;
  }
  public static int test(int a, int b){
    int a ;
    int b,c ;
    return a + b;
  }
  public static void show_result(int res){
    System.out.print(res);
  }

  public static void main(String[] args) {
    int sum;
    sum = test(a,b);
    show_result(sum);
  }
}
