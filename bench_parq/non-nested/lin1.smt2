(declare-var a (Array Int Int))
(declare-var a0 (Array Int Int))
(declare-var i Int)
(declare-var i1 Int)
(declare-var n Int)
(declare-rel inv ((Array Int Int) Int Int))
(declare-rel fail ())

(rule (inv a 0 n))

(rule (=> (and (inv a i n) (< i n)
  (= a0 (store a i i))
  (= i1 (+ i 1))) (inv a0 i1 n)))

(rule (=> (and (inv a i n) (not (< i n)) true) fail))

(query fail)

;
;void lin (int *a, int n) {
;  for (int i = 0; i < n; i++) {
;    a[i] = i;
;  }
;}
;

