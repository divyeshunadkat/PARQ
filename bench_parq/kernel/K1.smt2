(declare-var x (Array Int Int))
(declare-var x1 (Array Int Int))
(declare-var y (Array Int Int))
(declare-var z (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-var r Int)
(declare-var t Int)
(declare-var q Int)
(declare-rel inv ((Array Int Int) (Array Int Int) (Array Int Int) Int Int Int Int Int))
(declare-rel fail ())

(rule (inv x y z 0 r t q n))

(rule (=> (and (inv x y z i r t q n) (< i n)
  (= x1 (store x i (+ q (* (select y i) (+ (* r (select z (+ i 10))) (* t (select  z (+ i 11))) )))))
  (= i1 (+ i 1))) (inv x1 y z i1 r t q n)))

(rule (=> (and (inv x y z i r t q n) (not (< i n)) true) fail))

(query fail)


;
;    /*
;     *******************************************************************
;     *   K1 -- hydro fragment
;     *   Similar to K7 and K12
;     *******************************************************************
;     */
;
;     for ( i=0 ; i<n ; i++ ) {
;        x[i] = q + y[i]*( r*z[i+10] + t*z[i+11] );
;     }
;

