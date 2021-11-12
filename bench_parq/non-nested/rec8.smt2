(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var c Int)
(declare-var n Int)
(declare-rel inv0 ((Array Int Int) Int Int Int))
(declare-rel fail ())

(rule (inv0 a 0 c n))

(rule (=> (and (inv0 a i c n) (< i n)
  (= a0 (ite (= i 0) (store a i c) (store a i (select a (- i 1)))))
  (= i1 (+ i 1))) (inv0 a0 i1 c n)))

(rule (=> (and (inv0 a i c n) (not (< i n)) true) fail))

(query fail)

;
;void rec (int *a, int c, int n) {
;  for (int i = 0; i < n; i++) {
;    if ( i == 0 ) {
;      a[i] = c;
;    } else {
;      a[i] = a[i-1];
;    }
;  }
;}
;

