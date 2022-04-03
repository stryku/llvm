// RUN: %clang_cc1 -fsyntax-only -Wloop-analysis -verify -std=c++17 %s

struct S {
  bool stop() { return false; }
  bool keep_running;
};

void by_ref(int &value) { }
void by_value(int value) { }
void by_pointer(int *value) {}

void test1() {
  S s;
  for (; !s.stop();) {}
  for (; s.keep_running;) {}
  for (int i; i < 1; ++i) {}
  for (int i; i < 1; ) {}  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (int i; i < 1; ) { ++i; }
  for (int i; i < 1; ) { return; }
  for (int i; i < 1; ) { break; }
  for (int i; i < 1; ) { goto exit_loop; }
exit_loop:
  for (int i; i < 1; ) { by_ref(i); }
  for (int i; i < 1; ) { by_value(i); }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (int i; i < 1; ) { by_pointer(&i); }

  for (int i; i < 1; ++i)
    for (int j; j < 1; ++j)
      { }
  for (int i; i < 1; ++i)
    for (int j; j < 1; ++i)  // expected-warning {{variable 'j' used in loop condition not modified in loop body}}
      { }
  for (int i; i < 1; ++i)
    for (int j; i < 1; ++j)  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
      { }

  for (int *i, *j; i < j; ++i) {}
  for (int *i, *j; i < j;) {}  // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}

  // Dereferencing pointers is ignored for now.
  for (int *i; *i; ) {}
}

void test_while1() {
  S s;
  int i, j;
  while (!s.stop()) {}
  while (s.keep_running) {}
  while (i < 1) { ++i; }
  while (i < 1) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < 1) { return; }
  while (i < 1) { break; }
  while (i < 1) { goto exit_loop; }
  
exit_loop:
  while (i < 1) { by_ref(i); }
  while (i < 1) { by_value(i); } // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < 1) { by_pointer(&i); }

  while (i < 1) {
    while (j < 1) { ++j; }
    ++i;
  }
  while (i < 1) {
    while (j < 1) { ++i; } // expected-warning {{variable 'j' used in loop condition not modified in loop body}}
    ++i;
  }
  while (i < 1) {
    while (i < 1) { ++j; } // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
    ++i;
  }

  int *ip, *jp;
  while (ip < jp) { ++ip; }
  while (ip < jp) {} // expected-warning {{variables 'ip' and 'jp' used in loop condition not modified in loop body}}

  // Dereferencing pointers is ignored for now.
  while (*ip) {}
}

void test_do_while1() {
  S s;
  int i, j;
  do {} while (!s.stop());
  do {} while (s.keep_running);
  do { ++i; } while (i < 1);
  do {} while (i < 1); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { return; } while (i < 1);
  do { break; } while (i < 1);
  do { goto exit_loop; } while (i < 1);

exit_loop:
  do { by_ref(i); } while (i < 1);
  do { by_value(i); } while (i < 1); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { by_pointer(&i); } while (i < 1);

  do {
    do { ++j; } while (j < 1);
    ++i;
  } while (i < 1);

  do {
    do { ++i; } while (j < 1); // expected-warning {{variable 'j' used in loop condition not modified in loop body}}
    ++i;
  } while (i < 1);

  do {
    do { ++j; } while (i < 1); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
    ++i;
  } while (i < 1);

  int *ip, *jp;
  do { ++ip; } while (ip < jp);
  do {} while (ip < jp); // expected-warning {{variables 'ip' and 'jp' used in loop condition not modified in loop body}}

  // Dereferencing pointers is ignored for now.
  do {} while (*ip);
}

void test2() {
  int i, j, k;
  int *ptr;

  // Testing CastExpr
  for (; i; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i; ) { i = 5; }

  // Testing BinaryOperator
  for (; i < j; ) {} // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  for (; i < j; ) { i = 5; }
  for (; i < j; ) { j = 5; }

  // Testing IntegerLiteral
  for (; i < 5; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i < 5; ) { i = 5; }

  // Testing FloatingLiteral
  for (; i < 5.0; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i < 5.0; ) { i = 5; }

  // Testing CharacterLiteral
  for (; i == 'a'; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i == 'a'; ) { i = 5; }

  // Testing CXXBoolLiteralExpr
  for (; i == true; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i == true; ) { i = 5; }

  // Testing GNUNullExpr
  for (; ptr == __null; ) {} // expected-warning {{variable 'ptr' used in loop condition not modified in loop body}}
  for (; ptr == __null; ) { ptr = &i; }

  // Testing UnaryOperator
  for (; -i > 5; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; -i > 5; ) { ++i; }

  // Testing ImaginaryLiteral
  for (; i != 3i; ) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i != 3i; ) { ++i; }

  // Testing ConditionalOperator
  for (; i ? j : k; ) {} // expected-warning {{variables 'i', 'j', and 'k' used in loop condition not modified in loop body}}
  for (; i ? j : k; ) { ++i; }
  for (; i ? j : k; ) { ++j; }
  for (; i ? j : k; ) { ++k; }
  for (; i; ) { j = i ? i : i; }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i; ) { j = (i = 1) ? i : i; }
  for (; i; ) { j = i ? i : ++i; }

  // Testing BinaryConditionalOperator
  for (; i ?: j; ) {} // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  for (; i ?: j; ) { ++i; }
  for (; i ?: j; ) { ++j; }
  for (; i; ) { j = i ?: i; }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}

  // Testing ParenExpr
  for (; (i); ) { }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; (i); ) { ++i; }

  // Testing non-evaluated variables
  for (; i < sizeof(j); ) { }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i < sizeof(j); ) { ++j; }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  for (; i < sizeof(j); ) { ++i; }
}

void test_while2() {
  int i, j, k;
  int *ptr;

  // Testing CastExpr
  while (i) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i) { i = 5; }

  // Testing BinaryOperator
  while (i < j) {} // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  while (i < j) { i = 5; }
  while (i < j) { j = 5; }

  // Testing IntegerLiteral
  while (i < 5) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < 5) { i = 5; }

  // Testing FloatingLiteral
  while (i < 5.0) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < 5.0) { i = 5; }

  // Testing CharacterLiteral
  while (i == 'a') {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i == 'a') { i = 5; }

  // Testing CXXBoolLiteralExpr
  while (i == true) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i == true) { i = 5; }

  // Testing GNUNullExpr
  while (ptr == __null) {} // expected-warning {{variable 'ptr' used in loop condition not modified in loop body}}
  while (ptr == __null) { ptr = &i; }

  // Testing UnaryOperator
  while (-i > 5) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (-i > 5) { ++i; }

  // Testing ImaginaryLiteral
  while (i != 3i) {} // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i != 3i) { ++i; }

  // Testing ConditionalOperator
  while (i ? j : k) {} // expected-warning {{variables 'i', 'j', and 'k' used in loop condition not modified in loop body}}
  while (i ? j : k) { ++i; }
  while (i ? j : k) { ++j; }
  while (i ? j : k) { ++k; }
  while (i) { j = i ? i : i; } // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i) { j = (i = 1) ? i : i; }
  while (i) { j = i ? i : ++i; }

  // Testing BinaryConditionalOperator
  while (i ?: j) {} // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  while (i ?: j) { ++i; }
  while (i ?: j) { ++j; }
  while (i) { j = i ?: i; }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}

  // Testing ParenExpr
  while ((i)) { }  // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while ((i)) { ++i; }

  // Testing non-evaluated variables
  while (i < sizeof(j)) { } // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < sizeof(j)) { ++j; } // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  while (i < sizeof(j)) { ++i; }
}

void test_do_while2() {
  int i, j, k;
  int *ptr;

  // Testing CastExpr
  do {} while (i); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { i = 5; } while (i);

  // Testing BinaryOperator
  do {} while (i < j); // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  do { i = 5; } while (i < j);
  do { j = 5; } while (i < j);

  // Testing IntegerLiteral
  do {} while (i < 5); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { i = 5; } while (i < 5);

  // Testing FloatingLiteral
  do {} while (i < 5.0); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { i = 5; } while (i < 5.0);

  // Testing CharacterLiteral
  do {} while (i == 'a'); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { i = 5; } while (i == 'a');

  // Testing CXXBoolLiteralExpr
  do {} while (i == true); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do  { i = 5; } while (i == true);

  // Testing GNUNullExpr
  do {} while (ptr == __null); // expected-warning {{variable 'ptr' used in loop condition not modified in loop body}}
  do { ptr = &i; } while (ptr == __null);

  // Testing UnaryOperator
  do {} while (-i > 5); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { ++i; } while (-i > 5);

  // Testing ImaginaryLiteral
  do {} while (i != 3i); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { ++i; } while (i != 3i);

  // Testing ConditionalOperator
  do {} while (i ? j : k); // expected-warning {{variables 'i', 'j', and 'k' used in loop condition not modified in loop body}}
  do { ++i; } while (i ? j : k);
  do { ++j; } while (i ? j : k);
  do { ++k; } while (i ? j : k);
  do { j = i ? i : i; } while (i); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { j = (i = 1) ? i : i; } while (i);
  do { j = i ? i : ++i; } while (i);

  // Testing BinaryConditionalOperator
  do {} while (i ?: j); // expected-warning {{variables 'i' and 'j' used in loop condition not modified in loop body}}
  do { ++i; } while (i ?: j);
  do { ++j; } while (i ?: j);
  do { j = i ?: i; } while (i); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}

  // Testing ParenExpr
  do {} while ((i)); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { ++i; } while ((i));

  // Testing non-evaluated variables
  do {} while (i < sizeof(j)); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { ++j; } while (i < sizeof(j)); // expected-warning {{variable 'i' used in loop condition not modified in loop body}}
  do { ++i; } while (i < sizeof(j));
}

// False positive and how to silence.
void test3() {
  int x;
  int *ptr = &x;
  for (;x<5;) { *ptr = 6; }  // expected-warning {{variable 'x' used in loop condition not modified in loop body}}

  for (;x<5;) {
    *ptr = 6;
    (void)x;
  }
}

void test_while3() {
  int x;
  int *ptr = &x;
  while (x < 5) { *ptr = 6; }  // expected-warning {{variable 'x' used in loop condition not modified in loop body}}

  while (x < 5) {
    *ptr = 6;
    (void)x;
  }
}

void test_do_while3() {
  int x;
  int *ptr = &x;
  do { *ptr = 6; } while (x < 5); // expected-warning {{variable 'x' used in loop condition not modified in loop body}}

  do {
    *ptr = 6;
    (void)x;
  } while (x < 5);
}

// Check ordering and printing of variables.  Max variables is currently 4.
void test4() {
  int a, b, c, d, e, f;
  for (; a;);  // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  for (; a + b;);  // expected-warning {{variables 'a' and 'b' used in loop condition not modified in loop body}}
  for (; a + b + c;);  // expected-warning {{variables 'a', 'b', and 'c' used in loop condition not modified in loop body}}
  for (; a + b + c + d;);  // expected-warning {{variables 'a', 'b', 'c', and 'd' used in loop condition not modified in loop body}}
  for (; a + b + c + d + e;);  // expected-warning {{variables used in loop condition not modified in loop body}}
  for (; a + b + c + d + e + f;);  // expected-warning {{variables used in loop condition not modified in loop body}}
  for (; a + c + d + b;);  // expected-warning {{variables 'a', 'c', 'd', and 'b' used in loop condition not modified in loop body}}
  for (; d + c + b + a;);  // expected-warning {{variables 'd', 'c', 'b', and 'a' used in loop condition not modified in loop body}}
}

void test_while4() {
  int a, b, c, d, e, f;
  while (a);  // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  while (a + b);  // expected-warning {{variables 'a' and 'b' used in loop condition not modified in loop body}}
  while (a + b + c);  // expected-warning {{variables 'a', 'b', and 'c' used in loop condition not modified in loop body}}
  while (a + b + c + d);  // expected-warning {{variables 'a', 'b', 'c', and 'd' used in loop condition not modified in loop body}}
  while (a + b + c + d + e);  // expected-warning {{variables used in loop condition not modified in loop body}}
  while (a + b + c + d + e + f);  // expected-warning {{variables used in loop condition not modified in loop body}}
  while (a + c + d + b);  // expected-warning {{variables 'a', 'c', 'd', and 'b' used in loop condition not modified in loop body}}
  while (d + c + b + a);  // expected-warning {{variables 'd', 'c', 'b', and 'a' used in loop condition not modified in loop body}}
}

void test_do_while4() {
  int a, b, c, d, e, f;
  do {} while (a);  // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  do {} while (a + b);  // expected-warning {{variables 'a' and 'b' used in loop condition not modified in loop body}}
  do {} while (a + b + c);  // expected-warning {{variables 'a', 'b', and 'c' used in loop condition not modified in loop body}}
  do {} while (a + b + c + d);  // expected-warning {{variables 'a', 'b', 'c', and 'd' used in loop condition not modified in loop body}}
  do {} while (a + b + c + d + e);  // expected-warning {{variables used in loop condition not modified in loop body}}
  do {} while (a + b + c + d + e + f);  // expected-warning {{variables used in loop condition not modified in loop body}}
  do {} while (a + c + d + b);  // expected-warning {{variables 'a', 'c', 'd', and 'b' used in loop condition not modified in loop body}}
  do {} while (d + c + b + a);  // expected-warning {{variables 'd', 'c', 'b', and 'a' used in loop condition not modified in loop body}}
}

// Ensure that the warning doesn't fail when lots of variables are used
// in the conditional.
void test5() {
  for (int a; a+a+a+a+a+a+a+a+a+a;); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  for (int a; a+a+a+a+a+a+a+a+a+a+a;); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  for (int a; a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a;);  // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  for (int a; a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a;);//\
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
}

void test_while5() {
  int a;
  while (a+a+a+a+a+a+a+a+a+a); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  while (a+a+a+a+a+a+a+a+a+a+a); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  while (a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a);  // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  while (a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a);//\
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
}

void test_do_while5() {
  int a;
  do {} while (a+a+a+a+a+a+a+a+a+a); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  do {} while (a+a+a+a+a+a+a+a+a+a+a); // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  do {} while (a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a);  // \
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
  do {} while (a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a+a);//\
   // expected-warning {{variable 'a' used in loop condition not modified in loop body}}
}

// Ignore global variables and static variables.
int x6;
void test6() {
  static int y;
  for (;x6;);
  for (;y;);
}

void test_while6() {
  static int y;
  while (x6);
  while (y);
}

void test_do_while6() {
  static int y;
  do {} while (x6);
  do {} while (y);
}

void test7() {
  int i;
  for (;;i++) {  // expected-note{{incremented here}}
    if (true) test7();
    i++;  // expected-warning{{incremented both}}
  }
  for (;;i++) {  // expected-note{{incremented here}}
    if (true) break;
    ++i;  // expected-warning{{incremented both}}
  }
  for (;;++i) {  // expected-note{{incremented here}}
    while (true) return;
    i++;  // expected-warning{{incremented both}}
  }
  for (;;++i) {  // expected-note{{incremented here}}
    ++i;  // expected-warning{{incremented both}}
  }

  for (;;i--) {  // expected-note{{decremented here}}
    if (true) test7();
    i--;  // expected-warning{{decremented both}}
  }
  for (;;i--) {  // expected-note{{decremented here}}
    if (true) break;
    --i;  // expected-warning{{decremented both}}
  }
  for (;;--i) {  // expected-note{{decremented here}}
    while (true) return;
    i--;  // expected-warning{{decremented both}}
  }
  for (;;--i) {  // expected-note{{decremented here}}
    --i;  // expected-warning{{decremented both}}
  }

  // Don't warn when loop is only one statement.
  for (;;++i)
    i++;
  for (;;--i)
    --i;

  // Don't warn when loop has continue statement.
  for (;;i++) {
    if (true) continue;
    i++;
  }
  for (;;i--) {
    if (true) continue;
    i--;
  }

  // But do warn if the continue is in a nested loop.
  for (;;i--) { // expected-note{{decremented here}}
    for (int j = 0; j < 10; ++j) continue;
    i--; // expected-warning{{decremented both}}
  }
}

struct iterator {
  iterator operator++() { return *this; }
  iterator operator++(int) { return *this; }
  iterator operator--() { return *this; }
  iterator operator--(int) { return *this; }
};
void test8() {
  iterator i;
  for (;;i++) {  // expected-note{{incremented here}}
    if (true) test7();
    i++;  // expected-warning{{incremented both}}
  }
  for (;;i++) {  // expected-note{{incremented here}}
    if (true) break;
    ++i;  // expected-warning{{incremented both}}
  }
  for (;;++i) {  // expected-note{{incremented here}}
    while (true) return;
    i++;  // expected-warning{{incremented both}}
  }
  for (;;++i) {  // expected-note{{incremented here}}
    ++i;  // expected-warning{{incremented both}}
  }

  for (;;i--) {  // expected-note{{decremented here}}
    if (true) test7();
    i--;  // expected-warning{{decremented both}}
  }
  for (;;i--) {  // expected-note{{decremented here}}
    if (true) break;
    --i;  // expected-warning{{decremented both}}
  }
  for (;;--i) {  // expected-note{{decremented here}}
    while (true) return;
    i--;  // expected-warning{{decremented both}}
  }
  for (;;--i) {  // expected-note{{decremented here}}
    --i;  // expected-warning{{decremented both}}
  }

  // Don't warn when loop is only one statement.
  for (;;++i)
    i++;
  for (;;--i)
    --i;

  // Don't warn when loop has continue statement.
  for (;;i++) {
    if (true) continue;
    i++;
  }
  for (;;i--) {
    if (true) continue;
    i--;
  }

  // But do warn if the continue is in a nested loop.
  for (;;i--) { // expected-note{{decremented here}}
    for (int j = 0; j < 10; ++j) continue;
    i--; // expected-warning{{decremented both}}
  }
}

int f(int);
void test9() {
  // Don't warn when variable is defined by the loop condition.
  for (int i = 0; int x = f(i); ++i) {}
}

void test_while9() {
  // Don't warn when variable is defined by the loop condition.
  int i = 0;
  while (int x = f(i)) { ++i; }
}

// Don't warn when decomposition variables are in the loop condition.
// TODO: BindingDecl's which make a copy should warn.
void test10() {
  int arr[] = {1, 2, 3};
  for (auto[i, j, k] = arr;;) { }
  for (auto[i, j, k] = arr; i < j; ++i, ++j) { }

  for (auto[i, j, k] = arr; i;) { }
  for (auto[i, j, k] = arr; i < j;) { }
  for (auto[i, j, k] = arr; i < j; ++arr[0]) { }

  int a = 1, b = 2;
  for (auto[i, j, k] = arr; a < b;) { }  // expected-warning{{variables 'a' and 'b' used in loop condition not modified in loop body}}
  for (auto[i, j, k] = arr; a < b; ++a) { }

  for (auto [i, j, k] = arr; i < a;) { }
  for (auto[i, j, k] = arr; i < a; ++a) { }
  for (auto[i, j, k] = arr; i < a; ++i) { }
  for (auto[i, j, k] = arr; i < a; ++arr[0]) { }
};

void test_while10() {
  int arr[] = {1, 2, 3};
  auto[i, j, k] = arr;
  while (i < j) { ++i; ++j; }

  while (i) {}
  while (i < j) {}
  while (i < j ) { ++arr[0]; }

  int a = 1, b = 2;
  while (a < b) {}  // expected-warning{{variables 'a' and 'b' used in loop condition not modified in loop body}}
  while (a < b ) { ++a; }

  while (i < a) {}
  while (i < a) { ++a; }
  while (i < a) { ++i; }
  while (i < a) { ++arr[0]; }
};

void test_do_while10() {
  int arr[] = {1, 2, 3};
  auto[i, j, k] = arr;
  do { ++i; ++j; } while(i < j);

  do {} while (i);
  do  {} while (i < j);
  do { ++arr[0]; } while (i < j );

  int a = 1, b = 2;
  do {} while (a < b);  // expected-warning{{variables 'a' and 'b' used in loop condition not modified in loop body}}
  do { ++a; } while (a < b );

  do {} while (i < a);
  do { ++a; } while (i < a);
  do { ++i; } while (i < a);
  do { ++arr[0]; } while (i < a);
};
