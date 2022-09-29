
(define (resetScene)
 	(define sceneidToReset (lsscene-name "reset"))
 	(if sceneidToReset
 		(reset-scene sceneidToReset)
 		(format #t "no scene to reset\n")
 	)
)

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
  	(resetScene)
  )
)


