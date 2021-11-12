(declare-var x (Array Int Int))
(declare-var x1 (Array Int Int))
(declare-var u (Array Int Int))
(declare-var y (Array Int Int))
(declare-var z (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-var r Int)
(declare-var t Int)
(declare-rel inv ((Array Int Int) (Array Int Int) (Array Int Int) (Array Int Int) Int Int Int Int))
(declare-rel fail ())

(rule (inv x u y z 0 r t n))

(rule (=> (and (inv x u y z i r t n) (< i n)
  (= x1 (store x i (+ (select u i) (* r (+ (select z i) (* r (select  y i))))
                      (* t (+ (select u (+ i 3)) (* r (+ (select u (+ i 2)) (* r (select u (+ i 1))) )) ))
                      (* t (+ (select u (+ i 6)) (* r (+ (select u (+ i 5)) (* r (select u (+ i 4))) )) ))
                   ) ))
  (= i1 (+ i 1))) (inv x1 u y z i1 r t n)))

(rule (=> (and (inv x u y z i r t n) (not (< i n)) true) fail))

(query fail)


;
;    /*
;     *******************************************************************
;     *   K7 -- equation of state fragment
;     *   Similar to K12
;     *   Number of access outside the chunk are still finite.
;     *   These access can be converted to fresh scalars and computed
;     *   outside the loop.
;     *******************************************************************
;     */
;
;     for ( i=0 ; i<n ; i++ ) {
;        x[i] = u[i] + r*( z[i] + r*y[i] ) +
;               t*( u[i+3] + r*( u[i+2] + r*u[i+1] ) +
;               t*( u[i+6] + r*( u[i+5] + r*u[i+4] ) ) );
;     }
;


