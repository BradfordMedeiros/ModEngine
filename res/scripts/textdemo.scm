(define letters "a b c d e\n\nf g h i j\n\nk l m n o\n\np q r s t\n\nu v w x y\n\nz" )
(define specialKeys "! @ # $ %\n\n^ & * ( )\n\n_ + -")
(define numbers "1 2 3 4 5\n\n6 7 8 9 0\n\n")

(define (onFrame)
	(draw-text-ndi 
		(string-append 
			letters 
			"\n\n\n"
			numbers
			"\n\n\n"
			specialKeys
		)
		-0.5 0.5 20
	)
)

