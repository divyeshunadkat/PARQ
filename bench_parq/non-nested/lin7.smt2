(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var b (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-var C Int)
(declare-rel inv ((Array Int Int) (Array Int Int) Int Int Int))
(declare-rel fail ())

(rule (inv a b C 0 n))

(rule (=> (and (inv a b C i n) (< i n)
  (= a0 (ite (and (> C 0) (< C 3) (>= i C)) (store a i (- (select b i) (select b (- i C)))) (store a i (select b i))))
  (= i1 (+ i 1))) (inv a0 b C i1 n)))

(rule (=> (and (inv a b C i n) (not (< i n)) true) fail))

(query fail)

;
;void lin (int *a, int *b, int C, int n) {
;  for (int i = 0; i < n; i++) {
;    if ( C>0 && C<3 && i>=C ) {
;      a[i] = b[i] - b[i-C];
;    } else {
;      a[i] = b[i];
;    }
;  }
;}
;

