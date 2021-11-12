(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i0 Int)
(declare-var j Int)
(declare-var j0 Int)
(declare-var N Int)
(declare-rel inv0 ((Array Int Int) Int Int))
(declare-rel inv1 ((Array Int Int) Int Int Int))
(declare-rel fail ())

(rule (inv0 a 0 N))

(rule (=> (and (inv0 a i N) (< i N) (= a0 (store a i 0))) (inv1 a0 i 0 N)))

(rule (=> (and (inv1 a i j N) (< j N) (= a0 (store a i (+ j (select a i)))) (= j0 (+ j 1))) (inv1 a0 i j0 N)))

(rule (=> (and (inv1 a i j N) (not (< j N)) (= a0 (store a i (* 2 (select a i)))) (= i0 (+ i 1))) (inv0 a0 i0 N)))

(rule (=> (and (inv0 a i N) (not (< i N)) true) fail))

(query fail)


;
;void nested11(int *a, int N) {
;  for(i=0; i<N; i++) {
;    a[i] = 0;
;    for(j=0; j<N; j++) {
;      a[i] = a[i] + j;
;    }
;    a[i] = a[i] * 2;
;  }
;}
;
;


