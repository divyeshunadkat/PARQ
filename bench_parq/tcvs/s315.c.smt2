(declare-rel loop ((Array Int Int) Int Int Int Int ))
(declare-rel exit ())
(declare-var i Int )
(declare-var i1 Int )
(declare-var x Int )
(declare-var x1 Int )
(declare-var index Int )
(declare-var index1 Int )
(declare-var index_limit Int )
(declare-var a_i Int )
(declare-var count Int )
(declare-var a_array (Array Int Int) )

(rule (=> 
	(and 
		(= x (select a_array 0))
		(= index 0)
		(= index_limit (* count 8))
		(= i 0)
	)
	(loop a_array i index_limit x index)
))
(rule (=> 
	(and 
		(loop a_array i index_limit x index)
		(< i index_limit)
		(= a_i (select a_array i))
		(= x1 (ite (> a_i x) a_i x))
		(= index1 (ite (> a_i x) i index))
		(= i1 (+ i 1))
	)
	(loop a_array i1 index_limit x1 index1)
))
(rule (=> 
	(and 
		(loop a_array i index_limit x index)
		(not (< i index_limit))
	)
	exit
))
(query exit)