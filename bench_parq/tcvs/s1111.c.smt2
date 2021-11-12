(declare-rel loop ((Array Int Int) (Array Int Int) (Array Int Int) (Array Int Int) Int Int ))
(declare-rel exit ())
(declare-var i Int )
(declare-var i1 Int )
(declare-var t0 Int )
(declare-var t1 Int )
(declare-var t2 Int )
(declare-var t3 Int )
(declare-var index_limit Int )
(declare-var a_2i Int )
(declare-var b_i Int )
(declare-var c_i Int )
(declare-var d_i Int )
(declare-var count Int )
(declare-var a_array (Array Int Int) )
(declare-var a_array_new (Array Int Int) )
(declare-var b_array (Array Int Int) )
(declare-var c_array (Array Int Int) )
(declare-var d_array (Array Int Int) )

(rule (=> 
	(and 
		(= index_limit (* count 4))
		(= i 0)
	)
	(loop a_array b_array c_array d_array i index_limit)
))
(rule (=> 
	(and 
		(loop a_array b_array c_array d_array i index_limit)
		(< i index_limit)
		(= b_i (select b_array i))
		(= c_i (select c_array i))
		(= d_i (select d_array i))
		(= t0 (* c_i b_i))
		(= t1 (* d_i b_i))
		(= t2 (* c_i c_i))
		(= t3 (* d_i c_i))
		(= a_2i (+ (+ (+ (+ t0 t1) t2) t1) t3))
		(= a_array_new (store a_array (* 2 i) a_2i))
		(= i1 (+ i 1))
	)
	(loop a_array_new b_array c_array d_array i1 index_limit)
))
(rule (=> 
	(and 
		(loop a_array b_array c_array d_array i index_limit)
		(not (< i index_limit))
	)
	exit
))
(query exit)