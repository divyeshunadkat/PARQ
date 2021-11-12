(declare-var x0 (Array Int Int))
(declare-var x1 (Array Int Int))
(declare-var x2 (Array Int Int))
(declare-var y (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv0 ((Array Int Int) (Array Int Int) Int Int))
(declare-rel inv1 ((Array Int Int) (Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv0 x0 y 1 n))

(rule (=> (and (inv0 x0 y 1 n)
               (= x1 (store x0 0 (select y 0))))
          (inv1 x1 y i1 n)))

(rule (=> (and (inv1 x1 y i n) (< i n)
  (= x2 (store x1 i (+ (select x1 (- i 1)) (select y i)) ))
  (= i1 (+ i 1))) (inv1 x2 y i1 n)))

(rule (=> (and (inv1 x1 y i n) (not (< i n)) true) fail))

(query fail)


;
;    /*
;     *******************************************************************
;     *   K11 -- first sum
;     *   Has a dependence on the previous iteration of the loop.
;     *   If we can discover an invariant on y then we can try to
;     *   break the dependence.
;     *******************************************************************
;     */
;
;     x[0] = y[0];
;     for ( i=1 ; i<n ; i++ ) {
;        x[i] = x[i-1] + y[i];
;     }
;

