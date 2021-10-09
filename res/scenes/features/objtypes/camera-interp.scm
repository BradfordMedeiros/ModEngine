
(define oncamera1 #t)

(define (next-camera)
  (define activecam (if oncamera1 ">camera2" ">camera1"))
  (set-camera (gameobj-id (lsobj-name activecam)) 2)
  (set! oncamera1 (not oncamera1))
  (format #t "active camera: ~a\n" activecam)
)



(define (onMouse button action mods)
  (if (and (= button 0) (= action 0))
    (next-camera)
  )
)