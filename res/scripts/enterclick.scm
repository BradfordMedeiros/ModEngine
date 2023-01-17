
(define (onKey key scancode action mods)
	(format #t "key is: ~a\n" key)
  (if (and (= key 257) (= action 1))  ; enter
  	(begin
  		(format #t "enter click called\n")
  		(click)
  	)
  )
  (if (and (= key 344) (= action 1))  ; shift
  	(begin
  		(format #t "move mouse called\n")
  		(move-mouse (list -0.5 0.5))
  	)
  )
)