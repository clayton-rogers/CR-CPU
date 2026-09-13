
// foo is technically illegal because it's
// a redefinition of x. This compiler allows
// it because it considers the "compount-statement"
// that is the body of the funtion to be its own
// scope when it isn't really. So foo is illegal, but
// bar isn't.

//int foo(int x) {
//    int x = 3;
//}
//
//int bar(int x) {
//	{
//		int x = 3;
//	}
//}
//
//int main() {
//    foo(1);
//}