(declare-var x (Array Int Int))
(declare-var x1 (Array Int Int))
(declare-var y (Array Int Int))
(declare-var c Int)
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv ((Array Int Int) (Array Int Int) Int Int Int))
(declare-rel fail ())

(rule (inv x y c 0 n))

(rule (=> (and (inv x y c i n) (< i n)
  (= x1 (store x i (- (select y (+ i c)) (select y (- i c))) ))
  (= i1 (+ i 1))) (inv x1 y c i1 n)))

;(rule (=> (and (inv x y c i n) (not (< i n))
;  (<= 0 i1) (< i1 n) (not (= (select x i1) (- (select y (+ i1 c)) (select y (- i1 c)))))) fail))

(rule (=> (and (inv x y c i n) (not (< i n)) true) fail) )

(query fail)

;
;    /*
;     *******************************************************************
;     *   K12c -- first difference
;     *   This example does not have dependences across loop iterations.
;     *   However for the value of array x, there is a dependence on
;     *   another array.
;     *   If we assume that one cluster has access to x[0...k) and y[0...k)
;     *   then we are unable to compute x[k-1] as it access the value from
;     *   the next chunk which stores x[k...n)
;     *   But we will be able to compute the value of x[0...k-1) easily.
;     *   Value of x[k-1] can be computed outside the loop.
;     *******************************************************************
;     */
;
;     for ( i=0 ; i<n ; i++ ) {
;        x[i] = y[i+c] - y[i-c];
;     }
;
