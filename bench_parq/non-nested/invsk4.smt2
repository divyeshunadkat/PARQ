(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv0 ((Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv0 a 0 n))

(rule (=> (and (inv0 a 0 n)
  (= a0 (store a 0 (* 2 (+ n 1))))) (inv1 a0 0 n)))

(rule (=> (and (inv1 a i n) (< i n)
  (= a0 (store a (+ i 1) (- (select a i) 2)) )
  (= i1 (+ i 1))) (inv1 a0 i1 n)))

(rule (=> (and (inv1 a i n) (not (< i n)) true) fail))

(query fail)

;
;
;void invsk4 (int *a, int n) {
;  a[0] = 2*(n+1);
;  for (int i = 0; i < n; i++) {
;      a[i+1] = a[i] - 2;
;  }
;}
;
;
