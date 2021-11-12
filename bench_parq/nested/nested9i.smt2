(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i0 Int)
(declare-var j Int)
(declare-var j0 Int)
(declare-var N Int)
(declare-rel inv0 ((Array Int Int) Int Int))
(declare-rel inv1 ((Array Int Int) Int Int))
(declare-rel inv2 ((Array Int Int) Int Int Int))
(declare-rel inv3 ((Array Int Int) Int Int))
(declare-rel inv4 ((Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv0 a 0 N))

(rule (=> (and (inv0 a i N) (< i N) (= a0 (store a i 0)) (= i0 (+ i 1)) (inv0 a0 i N)))

(rule (=> (and (inv0 a i N) (not (< i N))) (inv1 a 0 N)))

(rule (=> (and (inv1 a i N) (< i N)) (inv2 a i 0 N)))

(rule (=> (and (inv2 a i j N) (< j i) (= a0 (store a j (+ 2 (select a j)))) (= j0 (+ j 1))) (inv2 a0 i j0 N)))

(rule (=> (and (inv2 a i j N) (not (< j i)) (= i0 (+ i 1))) (inv1 a i0 N)))

(rule (=> (and (inv1 a i N) (not (< i N))) (inv3 a 0 N)))

(rule (=> (and (inv3 a i N) (< i N) (= a0 (store a i (- i (select a i)))) (= i0 (+ i 1)) ) (inv3 a0 i0 N)))

(rule (=> (and (inv3 a i N) (not (< i N))) (inv4 a 0 N)))

(rule (=> (and (inv4 a i N) (< i N) (= a0 (store a i (- i (select a i)))) (= i0 (+ i 1)) ) (inv4 a0 i0 N)))

(rule (=> (and (inv4 a i N) (not (< i N)) true) fail))

(query fail)

;
;void nested9i(int *a, int N) {
;  for(i=0; i<N; i++) {
;    a[i] = 0;
;  }
;  for(i=0; i<N; i++) {
;    for(j=0; j<i; j++) {
;      a[j] = a[j] + 2;
;    }
;  }
;  for(i=0; i<N; i++) {
;    a[i] = a[i] - i;
;  }
;  for(i=0; i<N; i++) {
;    a[i] = a[i] - i;
;  }
;}
;
;


