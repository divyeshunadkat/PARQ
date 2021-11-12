(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var b (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-var C1 Int)
(declare-var C2 Int)
(declare-rel inv ((Array Int Int) (Array Int Int) Int Int Int Int))
(declare-rel fail ())

(rule (inv a b C1 C2 0 n))

(rule (=> (and (inv a b C1 C2 i n) (< i n)
  (= a0 (ite (and (<= 0 (- i C1)) (< (- i C1) n) (<= 0 (+ i C2)) (< (+ i C2) n) ) (store a i (- (select b (- i C1)) (select b (+ i C2)))) (store a i (select b i))))
  (= i1 (+ i 1))) (inv a0 b C1 C2 i1 n)))

(rule (=> (and (inv a b C1 C2 i n) (not (< i n)) true) fail))

(query fail)

;
;void lin (int *a, int *b, int C1, int C2, int n) {
;  for (int i = 0; i < n; i++) {
;    if ( 0<= i-C1 && i-C1 < n && 0 <= i+C2 && i+C2 < n ) {
;      a[i] = b[i-C1] - b[i+C2];
;    } else {
;      a[i] = b[i];
;    }
;  }
;}
;

