
(define look-velocity-x 0)
(define look-velocity-y 0)

(define boxupdown (lsobj-name "s>boxupdown"))
(define forwardvec (orientation-from-pos (list 0 0 0) (list 0 0 -1)))
(define (look elapsedTime) 
  (define deltax (* look-velocity-x 10 elapsedTime))
  (define deltay (* -1 look-velocity-y 10 elapsedTime))
  (gameobj-setrot! boxupdown (setfrontdelta forwardvec deltax deltay 0))
  (reset-look-velocity)
)
(define (reset-look-velocity) 
  (set! look-velocity-x 0)
  (set! look-velocity-y 0)
)
(define (onMouseMove x y)
  (set! look-velocity-x x)
  (set! look-velocity-y y)
)

(define box1 (mk-obj-attr "box1" 
  (list 
    (list "mesh" "../gameresources/build/primitives/walls/1-0.2-1.gltf")
    (list "position" (list 0 10 0))
    (list "texture" "./res/textures/wood.jpg")
    (list "physics" "disabled")
  )
))
;(make-parent box1 (gameobj-id boxupdown))

(define (onFrame)
  (define currpos (gameobj-pos (gameobj-by-id box1)))
  (define deltapos (list (* 10 (cos (time-seconds))) 0 (* 10 (sin (time-seconds)))))
  ;(format #t "box1 pos: ~a\n" currpos)
  (look (time-elapsed))
  (gameobj-setpos-rel! (gameobj-by-id box1) deltapos)
)

(set-camera (gameobj-id boxupdown))
;define_gsubr("gameobj-pos", 1, 0, 0, (void *)scmGetGameObjectPos);
;  scm_c_define_gsubr("gameobj-pos-world";

;  _define_gsubr("gameobj-setpos!", 2, 0, 0, (void *)setGameObjectPosition);
;  scm_c_define_gsubr("gameobj-setpos-rel!