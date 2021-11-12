(declare-var x (Array Int Int))
(declare-var x1 (Array Int Int))
(declare-var y (Array Int Int))
(declare-var z (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv ((Array Int Int) (Array Int Int) (Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv x y z 0 n))

(rule (=> (and (inv x y z i n) (< i n)
  (= x1 (store x i (* (select z i) (- (select y i) (select x (- i 1))))))
  (= i1 (+ i 1))) (inv x1 y z i1 n)))

(rule (=> (and (inv x y z i n) (not (< i n)) true) fail))

(query fail)


;
;    /*
;     *******************************************************************
;     *   K5 -- tri-diagonal elimination, below diagonal
;     *   Has a dependence on the previous iteration of the loop.
;     *   If we can discover an invariant on y & z then we can try to
;     *   break the dependence automatically.
;     *******************************************************************
;     */
;
;     for ( i=1 ; i<n ; i++ ) {
;        x[i] = z[i]*( y[i] - x[i-1] );
;     }
;


