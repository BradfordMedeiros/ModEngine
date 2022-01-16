
(define objid (gameobj-id mainobj))

(define (onFrame)
  (format #t "hello world\n")
)
(define (onMouse button action mods) 
  (format #t "about to remove self: ~a\n" objid)
  (rm-obj objid)
)
