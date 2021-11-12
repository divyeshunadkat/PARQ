(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv0 ((Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv0 a 2 n))

(rule (=> (and (inv0 a 2 n)
  (= a0 (store a 0 1)) (= a1 (store a0 1 2)) ) (inv1 a1 2 n)))

(rule (=> (and (inv1 a i n) (< i (* 2 n))
  (= a0 (store a i (+ (select a (- i 2)) 1)) )
  (= a1 (store a0 (+ i 1) (+ (select a (- i 1)) 2)) )
  (= i1 (+ i 2))) (inv1 a0 i1 n)))

(rule (=> (and (inv1 a i n) (not (< i (* 2 n))) true) fail))

(query fail)

;
;
;void invsk5 (int *a, int n) {
;  a[0] = 1;
;  a[1] = 2;
;  for (int i = 2; i < 2*n; i+=2) {
;      a[i] = a[i-2] + 1;
;      a[i+1] = a[i-1] + 2;
;  }
;}
;
;
