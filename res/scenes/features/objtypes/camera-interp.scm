(define currentcamera #f)

(define nextcam (list
  (list ">camera1" ">camera2")
  (list ">camera2" ">camera3")
  (list ">camera3" ">camera1")
))


(define (cycle-cameras currentcamera)
  (define cam (assoc currentcamera nextcam))
  (if cam
    (begin
      (format #t "next cam ~a\n" (cadr cam))
      (set-camera (gameobj-id (lsobj-name (cadr cam))) 5)

    )
  )
)

(define (onMouse button action mods)
  (if (and (= button 0) (= action 0)) (cycle-cameras))
)

(define (onCameraSystemChange camera default) 
  (format #t "camera is: ~a\n" camera)
  (set! currentcamera camera)
  (cycle-cameras camera)
)