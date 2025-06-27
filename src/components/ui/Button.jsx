import React from 'react'
import { motion } from 'framer-motion'
import clsx from 'clsx'

const variants = {
  primary: 'btn-primary',
  secondary: 'btn-secondary',
  success: 'bg-success-600 hover:bg-success-700 text-white',
  warning: 'bg-warning-600 hover:bg-warning-700 text-white',
  error: 'bg-error-600 hover:bg-error-700 text-white',
  ghost: 'bg-transparent hover:bg-gray-100 dark:hover:bg-gray-800 text-gray-700 dark:text-gray-300',
}

const sizes = {
  sm: 'px-3 py-1.5 text-sm',
  md: 'px-4 py-2',
  lg: 'px-6 py-3 text-lg',
}

export function Button({ 
  children, 
  variant = 'primary', 
  size = 'md',
  className,
  disabled,
  animate = true,
  ...props 
}) {
  const Component = animate ? motion.button : 'button'
  
  const motionProps = animate ? {
    whileHover: disabled ? {} : { scale: 1.02 },
    whileTap: disabled ? {} : { scale: 0.98 },
    transition: { duration: 0.1 }
  } : {}

  return (
    <Component
      className={clsx(
        'inline-flex items-center justify-center',
        'font-medium rounded-lg',
        'transition-all duration-200',
        'focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-primary-500',
        'disabled:opacity-50 disabled:cursor-not-allowed',
        variants[variant],
        sizes[size],
        className
      )}
      disabled={disabled}
      {...motionProps}
      {...props}
    >
      {children}
    </Component>
  )
}