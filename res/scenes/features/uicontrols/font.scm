(define (onFrame)
	(format #t "draw font placeholder\n")

	(draw-text "hello world" 600 600 4)	
	(draw-text-ndi "hello world" -0.5 0.5 4)	
)