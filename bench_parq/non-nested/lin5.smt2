(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var b (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv ((Array Int Int) (Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv a b 0 n))

(rule (=> (and (inv a b i n) (< i n)
  (= a0 (ite (< (+ i 1) n) (store a i (- (select b (+ i 1)) (select b i))) (store a i (select b i))))
  (= i1 (+ i 1))) (inv a0 b i1 n)))

(rule (=> (and (inv a b i n) (not (< i n)) true) fail))

(query fail)

;
;void lin (int *a, int *b, int n) {
;  for (int i = 0; i < n; i++) {
;    if ( i + 1 < n ) {
;      a[i] = b[i+1] - b[i];
;    } else {
;      a[i] = b[i];
;    }
;  }
;}
;

